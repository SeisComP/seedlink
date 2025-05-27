/***************************************************************************
 * Copyright (C) Tristan DIDIER, IPGP/OVSG, 2018                           *
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
#include <thread>

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


//*****************************************************************************
// Data Structures
//*****************************************************************************


//*****************************************************************************
// CLASS miscSerialProtocol
//*****************************************************************************


class miscSerialProtocol: public miscStringProtocol
  {

  private:
    // Attributs
    int SERIAL_CLOCK_PERIOD;//configurable with scconfig
    int RECVBUFSIZE;//depends on NCHAN

    int fd; //serial port file descriptor
    char* recvbuf;//Input Buffer
    char* end_buffer;// buffer's last byte
    bool stop_clock_thread=false;

    static miscSerialProtocol *obj;

    // Private methods
    void do_start();
    ssize_t read_port3(int fd, char* ptr, size_t n);
    void serial_clock();

  //Protected methods
  public:
    // Constructeur
    miscSerialProtocol(const string &myname);

    // Implementation of virtual methods from Proto class (cf serial_plugin.h)
    void start() override;
  };

miscSerialProtocol *miscSerialProtocol::obj;


//*******************************************************************************
// miscSerialProtocol class's methods
//*******************************************************************************

//########## constructeur ##########

miscSerialProtocol::miscSerialProtocol(const string &myname):miscStringProtocol(myname),SERIAL_CLOCK_PERIOD(dconf.serial_clock_period)
      {
	MAXFRAMELENGTH = 26+12*NCHAN+1; // dd/mm/yyyy HH:MM:SS.uuuuuu," -> 27 char; 1 channel 32 bits (from −2147483648 to 2147483647) -> max 11 char + 1 coma; end of line -> 1 char (\n)
	RECVBUFSIZE = 5*MAXFRAMELENGTH;
	recvbuf=new char[RECVBUFSIZE+1]; //5 frames + \0 sur dernier octet
	recvbuf[RECVBUFSIZE]='\0'; //
        end_buffer=recvbuf+RECVBUFSIZE;

        obj = this;
      }

//########## start ##########
//Open the serial port, launch serial_clock thread, and start to process incomming frames

void miscSerialProtocol::start()
  {
    fd = open_port(O_RDWR);//open port
    thread serial_clock_thread(&miscSerialProtocol::serial_clock,this);

    try
      {
        do_start();// main loop function
      }
    catch(PluginError &e)
      {
        seed_log << "closing device" << endl;
        close(fd);
	stop_clock_thread=true;
	serial_clock_thread.join();
        throw;
      }

    seed_log << "closing device" << endl;
    close(fd);
    stop_clock_thread=true;
    serial_clock_thread.join();
  }

//########## do_start ##########
// Get data from serial port and look for frames

void miscSerialProtocol::do_start()
  {
    char frame[MAXFRAMELENGTH];
    char* frame_start;
    char* frame_end;
    int frameLen;
    char* write_ptr=recvbuf;

    //Signals for plugin reboot
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sa.sa_flags = SA_RESTART;

    N(sigemptyset(&sa.sa_mask));
    N(sigaction(SIGALRM, &sa, NULL));

    //interruption every second (unblock select() in read_port3)
    struct itimerval itv;
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;

    N(setitimer(ITIMER_REAL, &itv, NULL));

    // MAIN LOOP STARTS HERE !
    while(!terminate_proc)
      {
	 //Get data from serial port
         write_ptr+=read_port3(fd, write_ptr, end_buffer-write_ptr);

         while(true){
	 	//Look for beginning and end of frame
	 	frame_start=strchr(recvbuf,10); //first LF in the buffer
	 	if (frame_start == NULL) break ;
	 	frame_end=strchr(frame_start+1,10); //second LF in the buffer
		if (frame_end == NULL) break ;
	 	frameLen=frame_end-frame_start;

	 	//extract frame
	 	strncpy(frame,frame_start+1,frameLen-1);
	 	frame[frameLen-1]='\0';

	 	//slide buffer
	 	strcpy(recvbuf,frame_end);
	 	write_ptr-=frame_end-recvbuf;

         	handle_response(frame); //process the frame with handle_response()
	 }
      }
  }

//########## serial_clock ##########
//Send timing frame to serial port every SERIAL_CLOCK_PERIOD if SERIAL_CLOCK_PERIOD > 0

void miscSerialProtocol::serial_clock()
{
	char time_str[100];
	struct timeval tv_now;
	struct timeval tv_nextRun;
	tv_nextRun.tv_usec=0;
	time_t time_t_now;
	struct tm *tm_now;

	if(SERIAL_CLOCK_PERIOD <= 0 )
		return;

	while(!stop_clock_thread){
		//sleep up to next sending
		gettimeofday(&tv_now,NULL);
		tv_nextRun.tv_sec=((tv_now.tv_sec/SERIAL_CLOCK_PERIOD)+1)*SERIAL_CLOCK_PERIOD;
		usleep((tv_nextRun.tv_sec-tv_now.tv_sec)*1000000-tv_now.tv_usec);

		//steps from tv_now to text
		time_t_now = tv_now.tv_sec;
		tm_now = localtime(&time_t_now);
		strftime(time_str,sizeof(time_str)-1, "%Y-%m-%d %H:%M:%S\n",tm_now);

		//print result
		write(fd,time_str,strlen(time_str));
	}

	return;
}

//########## read_port3 ##########

ssize_t miscSerialProtocol::read_port3(int fd, char* ptr, size_t n)
  {
    ssize_t nread;
    fd_set fds;
    struct timeval tv;
    bool carryOn=false;
    int res;

    while(carryOn==false){
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

        //timeout definition
	//Whatever, interruptions already unblock select() every second
        tv.tv_sec = 10;
        tv.tv_usec = 0;

        //Wait for data
    	if((res = select(fd + 1, &fds, NULL, NULL, &tv)) < 0){
            if(errno == EINTR){
		   continue;
	    }
            throw PluginLibraryError("select error");
    	}else{
	    if(!FD_ISSET(fd, &fds))//If no data available
		continue;

	    carryOn=true;
	}
     }

    nread = read(fd, ptr, n);
    ptr[nread]='\0';

    if(nread < 0) throw PluginReadError(dconf.port_name, strerror(errno));
    else if(nread == 0) throw PluginReadError(dconf.port_name);

    return(nread);
  }

//##########

RegisterProto<miscSerialProtocol> proto("miscSerial");

} // unnamed namespace

