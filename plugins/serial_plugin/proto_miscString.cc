/***************************************************************************
 * Copyright (C) Tristan DIDIER, IPGP/OVSG, 2020                           *
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


#include "proto_miscString.h"

//########## Constructeur ##########
miscStringProtocol::miscStringProtocol(const string &myname):NCHAN(dconf.channelsNumber), SAMPLE_PERIOD(dconf.sample_period), FLUSH_PERIOD(dconf.flush_period), miscString_channels(NCHAN),startup_message(true),soh_message(true), last_day(-1), last_soh(-1)
	{
     	// next forced flush date (only needed if FLUSH_PERIOD > 0)
	struct timeval tv;
     	N(gettimeofday(&tv, NULL));//get time machine
     	nextFlush = tv.tv_sec+FLUSH_PERIOD;//make timestamp of next flush enforcing
     	}

//########## alarm_handler ###########
//Used by reboot signals

void miscStringProtocol::alarm_handler(int sig)
  {
	//do nothing  
  }

//########## attach_output_channel ##########
//Register unpacked channels to seedlink server

void miscStringProtocol::attach_output_channel(const string &source_id,
  const string &channel_name, const string &station_name,
  double scale, double realscale, double realoffset, const string &realunit,
  int precision)
  {
    int n;
    char *tail;

    n = strtoul(source_id.c_str(), &tail, 10);
    
    if(*tail || n > NCHAN)
        throw PluginADInvalid(source_id, channel_name);

    if(n == NCHAN)
        return;
    
    if(miscString_channels[n] != NULL)
        throw PluginADInUse(source_id, miscString_channels[n]->channel_name);

    miscString_channels[n] = new OutputChannel(channel_name, station_name,
      dconf.zero_sample_limit, scale);
  }

//########## flush_channels ##########
// For all channels, force to send a SL dataBlock with the available data (the 512 octets data block is padded with zeros)

void miscStringProtocol::flush_channels()
  {
    for(int n = 0; n < NCHAN; ++n)
      {
        if(miscString_channels[n] != NULL)
            miscString_channels[n]->flush_streams();
      }
  }

//########### handle_response ##########
// This function aims at parsing timing part of the input string and at executing time related actions

void miscStringProtocol::handle_response(char* frame)
  {
    /*** Get machine time***/
    struct timeval tv;
    N(gettimeofday(&tv, NULL));//get time machine
    time_t t= tv.tv_sec;//get timestamp
    tm* ptm = gmtime(&t);//convert to tm struct

    /*** Extract miscString_time ***/
    time_t tt_miscStringTime;//timestamp for miscString_time
    tm tm_miscStringTime;//tm struct for miscString_time
    float seconds_float;
    int usec;
    char separator;

    if(sscanf(frame, "%d-%d-%d%c%d:%d:%f", &tm_miscStringTime.tm_year, &tm_miscStringTime.tm_mon, &tm_miscStringTime.tm_mday, &separator, &tm_miscStringTime.tm_hour, &tm_miscStringTime.tm_min, &seconds_float)<6)
    {
	    return;
    }

    tm_miscStringTime.tm_sec=int(seconds_float);
    usec=int(seconds_float*1000000)%1000000;
    tm_miscStringTime.tm_year-=1900;//to match tm struc def
    tm_miscStringTime.tm_mon-=1;//to match tm struct def
    tt_miscStringTime=mktime(&tm_miscStringTime);//convert to timestamp

    /*** Compare miscString_time with machine time***/
    char time_str[20];
    if(abs(t-tt_miscStringTime)>300){//if time difference more than 5 minutes
	    logs(LOG_WARNING) << "Time difference between seedlink machine and miscString is larger than 5 minutes" <<endl;
	    strftime(time_str,20,"%F %T",ptm);
	    logs(LOG_WARNING) << "Machine time : " << time_str << endl;
	    strftime(time_str,20,"%F %T",&tm_miscStringTime);
	    logs(LOG_WARNING) << "miscString time : " << time_str << endl;
	    return;

    }else{//If time consistency OK
	    //convert to EXT_TIME
	    EXT_TIME et;
	    et.year = tm_miscStringTime.tm_year+1900;
	    et.month = tm_miscStringTime.tm_mon+1;
	    et.day = tm_miscStringTime.tm_mday;
	    et.hour = tm_miscStringTime.tm_hour;
	    et.minute = tm_miscStringTime.tm_min;
	    et.second = tm_miscStringTime.tm_sec;
	    et.usec = usec;
	    et.doy = mdy_to_doy(et.month, et.day, et.year);
	    //Convert to INT_TIME
	    INT_TIME it = ext_to_int(et);

  
    /*** check if miscString time matches the expected sample time ***/

    if(!digitime.valid)
      {
        digitime.it = it;//reset digitime
        digitime.valid = true;
        digitime.exact = true;
      }
    else
      {
	int sec=floor(SAMPLE_PERIOD);
	int usec=(SAMPLE_PERIOD-sec)*1000000;	
        digitime.it = add_time(digitime.it, sec, usec);
        double time_diff = tdiff(it, digitime.it);

        DEBUG_MSG("time_diff = " << time_diff << endl);
        
        if(abs(time_diff) > MAX_TIME_ERROR)//if miscString time does not match expected sample time
          {
            logs(LOG_WARNING) << "time diff. " << time_diff / 1000000.0 << " sec" << endl;
            digitime.it = it;//Reset digitime.it
          }
      }

    // If new day -> send soh and startup message    
    if(digitime.it.second / 86400 != last_day)
      {
        startup_message = true;
        soh_message = true;
        last_day = digitime.it.second / 86400;
      } 
    
    // If time to send soh -> send soh
    if(dconf.statusinterval &&
      digitime.it.second / (dconf.statusinterval * 60) != last_soh)
      {
        soh_message = true;
        last_soh = digitime.it.second / (dconf.statusinterval * 60);
      }

    if(et.hour != 0 || et.minute != 0 || et.second != 0)//Copied from modbus plugin but I do not see the goal...
      {

	// send start_up msg
        if(startup_message)
          {
            startup_message = false;
            seed_log << ident_str << SEED_NEWLINE
                     << "protocol: " << dconf.proto_name << SEED_NEWLINE
                     << "sample rate: " << (1 / (double(SAMPLE_PERIOD)))
                     << " Hz" << endl;
          }
	
	//send soh message
        if(soh_message)
          {
            soh_message = false;
            strftime(time_str, 20, "%F;%T", ptm);
            seed_log << fixed << "status: " << time_str;
	    // To complete, or not...
	    seed_log << endl;
          }
      }
	
      decode_message(frame); //parse the remaining part of the input string

    //Force flush if needed
    if( FLUSH_PERIOD !=0 && t >= nextFlush){
        flush_channels();
        nextFlush=t+FLUSH_PERIOD;

    }
  }

  /// Update nextFlush date if no data comming
  bool all_channels_empty = true;

  for(int n = 0; n < NCHAN; ++n){
	if(miscString_channels[n] != NULL){
	    all_channels_empty = false;
	    break;
	}
  }

  if(all_channels_empty==true){
         nextFlush=t+FLUSH_PERIOD;
  }

}

//########## decode_message ##########
//This function aims at parsing channels'data

void miscStringProtocol::decode_message(char* data)
{
	DEBUG_MSG(msg << endl);
	
	char* tail= data;
	unsigned int offset;
	int idx=0;
	int val; 

	offset=strcspn(tail, ",")+1; //look for first coma
	tail+=offset;

	while(true){
		if(miscString_channels[idx] != NULL){
			if(sscanf(tail, "%d", &val)==1){//get following integer
				//and put it in corresponding channel vector
				miscString_channels[idx]-> set_timemark(digitime.it, 0, digitime.quality);
				miscString_channels[idx]->put_sample(val);
			}
		}

		offset=strcspn(tail, ",")+1;//Look for next coma

		if(offset==strlen(tail)+1){//if no more coma in input string
			break;
		}

		tail+=offset;
		idx++;
	}
}

