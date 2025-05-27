/***************************************************************************
 * Copyright (C) Tristan DIDIER, IPGP/OVSG, 2019                           *
 * All rights reserved.                                                    *
 *                                                                         *
 * GNU Affero General Public License Usage                                 *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/

#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "qtime.h"
#include "utils.h"
#include "cppstreams.h"
#include "serial_plugin.h"
#include "plugin_channel.h"
#include "diag.h"
#include "big-endian.h"
#include "proto_miscString.h"

using namespace std;
using namespace Utilities;
using namespace CPPStreams;
using namespace SeedlinkPlugin;

namespace {


//****************************************************************************
// CONSTANTES
//****************************************************************************

const int MAX_ERROR_MSG_SIZE = 100;

//*****************************************************************************
// Data Structures
//*****************************************************************************


//*****************************************************************************
// CLASS miscScript_Protocol
//*****************************************************************************


class miscScriptProtocol: public miscStringProtocol
  {

  private:
    // Attributs
    string SCRIPT_PATH;//configurable with scconfig
    string SCRIPT_ARGS;//configurable with scconfig

    int pipefd_data[2]; //file descriptors to pipe stdout of child
    int pipefd_error[2]; //file descriptors to pipe stderr of child
    pid_t cpid; // child pid

    static miscScriptProtocol *obj;

    //Private methods
    void do_start();
    char** prepare_args(string path, string args);

  public:
    // Constructeur
    miscScriptProtocol(const string &myname);

    // Implementation of virtual methods from Proto class (cf serial_plugin.h)
    void start() override;
  };

miscScriptProtocol *miscScriptProtocol::obj;


//*******************************************************************************
// Methodes de la classe miscScriptProtocol
//*******************************************************************************

//########## constructeur ##########

   miscScriptProtocol::miscScriptProtocol(const string &myname):miscStringProtocol(myname),SCRIPT_PATH(dconf.script_path),SCRIPT_ARGS(dconf.script_args)
      {
	MAXFRAMELENGTH = 27+12*NCHAN+1; //dd/mm/yyyy HH:MM:SS.uuuuuu," -> 23 caracteres; channel 32 bits (de −2147483648 à 2147483647) -> max 11 caractères + 1 virgule; fin de ligne -> un caractère (\n)

        obj = this;
      }

//########## start ##########
//launch script and launch the processing of incomming frames

void miscScriptProtocol::start()
  {
	 /////Create Pipes////
	 if (pipe(pipefd_data) == -1) {
	     	throw PluginLibraryError("pipe data error");;
	 }

	 if (pipe(pipefd_error) == -1) {
	 	throw PluginLibraryError("pipe error error");
	 }

	 ///// Fork /////
	 cpid = fork();
	 if (cpid == -1) {
	 	throw PluginLibraryError("fork error");
	 }

	 ///// I am Child /////
	 if (cpid == 0) {
	  	close(pipefd_data[0]);//no need to read on this side
	  	close(pipefd_error[0]);//no need to read on this side
		int stdout_orig=dup(STDOUT_FILENO);	//save original stdout
		int stderr_orig=dup(STDERR_FILENO);	//save original stderr
	        while ((dup2(pipefd_data[1], STDOUT_FILENO) == -1) && (errno == EINTR)) {} //redirect stdout to pipe
	  	while ((dup2(pipefd_error[1], STDERR_FILENO) == -1) && (errno == EINTR)) {} //redirect stderr to pipe
	        close(pipefd_data[1]); // fd no longer needed - the dup'ed handles are sufficient
	        close(pipefd_error[1]); // fd no longer needed - the dup'ed handles are sufficient

		execv(SCRIPT_PATH.c_str(),prepare_args(SCRIPT_PATH,SCRIPT_ARGS));//Run script

		//in case of problem with execv :
		while ((dup2(stdout_orig, STDOUT_FILENO) == -1) && (errno == EINTR)) {} //restore stdout
		while ((dup2(stderr_orig, STDERR_FILENO) == -1) && (errno == EINTR)) {} //restore stderr
		logs(LOG_ALERT) << "execv problem : an error occurred while running script" <<endl;//in case of script problem

       	///// I am Parent /////
    	 }else{
		close(pipefd_data[1]); // no need to write on this side
		close(pipefd_error[1]); // no need to write on this side

    		try{
        		do_start();// main loop function
      		}catch(PluginError &e){
        		seed_log << "closing device" << endl;
        		throw;
		}
      	}
  }

//########## do_start ##########
// Get data from script and look for frames

void miscScriptProtocol::do_start()
  {
    char frameData[MAXFRAMELENGTH];
    char buf_error[MAX_ERROR_MSG_SIZE];
    ssize_t nread;
    fd_set fds;
    struct timeval tv;
    int status;

    //Signals for plugin reboot
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;

    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGALRM, &sa, NULL));

    //interruption every seconds (unblock select())
    struct itimerval itv;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    N(setitimer(ITIMER_REAL, &itv, NULL));

    // MAIN LOOP STARTS HERE !
    while((!terminate_proc) && (waitpid(cpid, &status, WNOHANG)==0)){

	//Initialize files descriptors set for select
	FD_ZERO(&fds);
	FD_SET(pipefd_data[0], &fds);
	FD_SET(pipefd_error[0], &fds);

	//timeout definition
	//Whatever, interruptions already unblock select() every second
	tv.tv_sec = 10;
	tv.tv_usec = 0;

	// wait for data or error msg, or timer interruption
	if (select(max(pipefd_data[0],pipefd_error[0])+1, &fds, NULL, NULL, &tv)<0){
		if(errno == EINTR){//Si debloqué par timer
			continue;
		}else{
			throw PluginLibraryError("select error");
		}
	}

	//if data available
	if(FD_ISSET(pipefd_data[0], &fds)){
		nread=read(pipefd_data[0], &frameData, MAXFRAMELENGTH-1);
		frameData[nread]='\0';
		logs(LOG_DEBUG)<< "new frame : "<<string(frameData)<<endl;
		handle_response(frameData);
	}

	//if error msg available
	if(FD_ISSET(pipefd_error[0], &fds)){
		nread=read(pipefd_error[0], &buf_error, MAX_ERROR_MSG_SIZE-1);
		buf_error[nread]='\0';
		if (strcmp(buf_error,"")!=0){logs(LOG_WARNING) << "script error msg : " << string(buf_error) << endl;}
	 }
      }
  }


//########## prepare_args ##########
//convert script + args to char** (argv[] style)

char** miscScriptProtocol::prepare_args(string path,string args)
{
 	string buff="";
	vector<string> v;
    	char separator=' ';//space
	char c;

	//argv[0] contains script path
	path+='\0';
	v.push_back(path);

	//Then split args
	args+=separator;
	for(unsigned int i=0; i<args.length(); i++){
		c=args[i];
		if(c != separator){
	    		buff+=c;
		}else if(buff != ""){
			buff+='\0';
			v.push_back(buff);
			buff = "";
		}
	}

	// Convert to char**
	char** argsTab=new char*[v.size()+1];// +1 for the last element (NULL) added at the end
	int i=0;
	string str;

	for(vector<string>::iterator it = v.begin() ; it != v.end(); ++it){
		str=*it;
		argsTab[i]=strdup((char*)str.c_str());
		i++;
	}

	//Set last tab element to NULL
	//argsTab[i]=new char[1];
	argsTab[i]=(char*)NULL;

	//return resultat;
	return argsTab;
}

//##########

RegisterProto<miscScriptProtocol> proto("miscScript");

} // unnamed namespace

