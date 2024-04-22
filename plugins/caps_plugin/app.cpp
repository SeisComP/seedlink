/***************************************************************************
 * seedlink caps_plugin
 * Copyright (C) 2016  gempa GmbH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ***************************************************************************/


#include "app.h"

#include <gempa/caps/packet.h>
#include <gempa/caps/rawpacket.h>
#include <gempa/caps/mseedpacket.h>
#include <gempa/caps/sessiontable.h>
#include <gempa/caps/socket.h>
#include <gempa/caps/utils.h>

#include <gempa/caps/log.h>

#include <plugin.h>
#include <libslink.h>

#include <boost/bind.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>

#include <algorithm>
#include <errno.h>
#include <sstream>



using namespace std;
using namespace boost::program_options;


#define CHECK(x) if ( x <= 0 ) LogError("Error transmitting data")

namespace {

#define LOG_CAPS_CHANNEL(fmt) \
	va_list ap;\
	va_start(ap, fmt);\
	vfprintf(stderr, fmt, ap); fprintf(stderr, "\n");\
	va_end(ap)

inline void LogError(const char *fmt, ...) {
	LOG_CAPS_CHANNEL(fmt);
}

inline void LogWarning(const char *fmt, ...) {
	LOG_CAPS_CHANNEL(fmt);
}

inline void LogNotice(const char *fmt, ...) {
	LOG_CAPS_CHANNEL(fmt);
}

inline void LogInfo(const char *fmt, ...) {
	LOG_CAPS_CHANNEL(fmt);
}

inline void LogDebug(const char *fmt, ...) {
	LOG_CAPS_CHANNEL(fmt);
}


class FlagCounter: public boost::program_options::untyped_value {
	public:
		FlagCounter(unsigned int* count);
		void xparse(boost::any&, const vector<string>&) const;

	private:
		unsigned int* _count;
};

FlagCounter::FlagCounter(unsigned int* count)
    : boost::program_options::untyped_value(true), _count(count) {
}

void FlagCounter::xparse(boost::any&, const vector<string>&) const {
	++(*_count);
}



// The following wildcmp functions are taken from
// trunk/src/libs/seiscomp/core/strings.cpp.
bool wildcmp(const char *pat, const char *str) {
	const char *s, *p;
	bool star = false;

loopStart:
	for ( s = str, p = pat; *s; ++s, ++p ) {
		switch ( *p ) {
			case '?':
				break;
			case '*':
				star = true;
				str = s, pat = p;
				do { ++pat; } while (*pat == '*');
				if ( !*pat ) return true;
				goto loopStart;
			default:
				if ( *s != *p )
					goto starCheck;
				break;
		} /* endswitch */
	} /* endfor */

	while (*p == '*') ++p;

	return (!*p);

starCheck:
	if ( !star ) return false;
	++str;
	goto loopStart;
}


bool wildcmp(const std::string &wild, const std::string &str) {
	return wildcmp(wild.c_str(), str.c_str());
}

const int MAX_SAMPLES = ((SLRECSIZE - 64) * 2);


}


namespace Gempa {
namespace  CAPS {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

typedef void (*FUNC_PTR)(const char*, ...);
struct LogChannel {
	LogChannel(LogLevel level = LL_UNDEFINED, FUNC_PTR f = nullptr)
	    : level(level), f(f) {}

	LogLevel level;
	FUNC_PTR f;
};

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::App(int argc, char** argv) : Application(argc, argv) {
	_exitRequested = false;
	_sessionTable.setItemAddedFunc(boost::bind(&App::itemAdded, this, _1));
	_sessionTable.setItemAboutToBeRemovedFunc(boost::bind(&App::itemAboutToBeRemoved, this, _1));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::exit(int returnCode) {
	Application::exit(returnCode);
	if ( _socket && _socket->isValid() ) {
		_socket->close();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
App::Request *App::findRequest(const string &streamID) {
	for ( const auto &item : _requests) {
		if ( wildcmp(item->streamID, streamID) ) {
			return item;
		}
	}

	return nullptr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::init() {
	if ( !Application::init() ) {
		return false;
	}

	if ( !validateParameters() ) {
		return false;
	}

	_maximumTimeDiff = TimeSpan(_pMaximumTimeDiff);

	if ( _verbosity > 0 ) {
		vector<LogChannel> channels;
		channels.push_back(LogChannel(Gempa::CAPS::LL_ERROR, LogError));
		channels.push_back(LogChannel(Gempa::CAPS::LL_WARNING, LogWarning));
		channels.push_back(LogChannel(Gempa::CAPS::LL_INFO, LogInfo));
		channels.push_back(LogChannel(Gempa::CAPS::LL_DEBUG, LogDebug));

		for ( unsigned int i = 0; i < _verbosity; ++i ) {
			LogChannel &ch = channels[i];
			SetLogHandler(ch.level, ch.f);
		}
	}

	// Read last states
	if ( !_logFile.empty() ) {
		CAPS_DEBUG("Reading states from file %s", _logFile.c_str());
		ifstream ifs;
		ifs.open(_logFile.c_str());
		if ( !ifs.is_open() )
			LogError("Unable to load last state from file %s", _logFile.c_str());
		else {
			std::string streamID, time;
			while ( ifs.good() ) {
				streamID.clear();
				time.clear();
				ifs >> streamID >> time;
				if ( streamID.empty() ) {
					continue;
				}

				Time ts;
				if ( !ts.fromString(time.c_str(), "%FT%T.%fZ") ) {
					continue;
				}

				_states[streamID] = ts;
			}
		}
	}

	if ( !_startTime.valid() ) {
		_startTime = Time::GMT();
	}

	CAPS_INFO("Recovered %d states", (int)_states.size());

	if ( !_streamsFile.empty() && !readStreams(_streamsFile) ) {
		return false;
	}

	for ( size_t i = 0; i < _streams.size(); ++i ) {
		vector<string> toks;
		const char *src = _streams[i].c_str();
		boost::split(toks, src, boost::is_any_of("."), boost::token_compress_off);
		if ( toks.size() != 4 ) {
			LogError("%s: wrong id: expected four items separated by .",
			           _streams[i].c_str());
			return false;
		}

		addRequest(toks[0], toks[1], toks[2], toks[3],
		           _startTime, _endTime, {}, false);
	}


	for ( const auto &item : _states ) {
		Request *req = findRequest(item.first);
		if ( req ) {
			vector<string> toks;
			const char *src = item.first.c_str();
			boost::split(toks, src, boost::is_any_of("."), boost::token_compress_off);
			addRequest(toks[0], toks[1], toks[2], toks[3],
			           item.second, Time(), {}, true);
		}
	}


	if ( _requests.empty() ) {
		CAPS_DEBUG("No requests given, nothing to do");
		return false;
	}

	_socket = SocketPtr(_ssl ? new SSLSocket : new Socket);
	_socketBuf.setsocket(_socket.get());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::initCommandLine() {
	_verbosity = 0;

	_options = OptionsPtr(new boost::program_options::options_description());
	options_description generic("Generic");
	generic.add_options()
	    ("help,h", "produce help message")
	    ("verbosity", value(&_verbosity), "Verbosity level [0..4]");

	(options_description_easy_init(&generic))("v,v", new FlagCounter(&_verbosity),
	                                          "Increase verbosity level (may be repeated, eg. -vv)");

	options_description journal("Journal");
	journal.add_options()
	    ("journal,j", value(&_logFile), "File to store stream states. "
	     "Use an empty string to disable this feature.");

	options_description mode("Mode");
	mode.add_options()
	    ("archive", "Disable realtime mode")
	    ("dump", "Dump all received data to stdout and don't use the output port")
	    ("in-order", "Request all records in-order. Out-of-order records will be skipped")
	    ("max-time-diff", value(&_pMaximumTimeDiff),
	     "The maxmimum time difference with respect to current time of the end "
	     "time of a received record. If exceeded then the end time will not be "
	     "logged into the state file.");

	options_description streams("Streams");
	streams.add_options()
	    ("input,I", value(&_address), "Data input host [[caps|capss]://][user:pass@]host[:port]")
	    ("add-stream,A", value(&_streams), "StreamID [net.sta.loc.cha] to add")
	    ("stream-file,f", value(&_streamsFile), "Stream configuration file")
	    ("begin", value(&_strStartTime), "Start time of data time window")
	    ("end", value(&_strEndTime), "End time of data time window");


	_options->add(generic).add(journal).add(mode).add(streams);

	_vm.clear();

	try {
		store(parse_command_line(_argc, _argv, *_options), _vm);
		notify(_vm);
	}
	catch ( exception &e ) {
		cerr << e.what() << endl;
		cerr << *_options << "\n";
		return false;
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::readStreams(const string &filename) {
	ifstream ifs(filename.c_str());
	if ( !ifs.is_open() ) {
		LogError("Failed to open streams from file %s", filename.c_str());
		return false;
	}

	string streamID, line;
	int line_number = 0;
	while ( getline(ifs, line) ) {
		++line_number;

		if ( !line.empty() && line[0] == '#' ) {
			continue;
		}

		std::vector<std::string> toks;
		const char *src = line.c_str();
		boost::split(toks, src, boost::is_any_of(" "),
		             boost::token_compress_off);

		const std::string &streamID = toks[0];
		Unpack unpack;
		if ( toks.size() > 1 ) {
			src = toks[1].c_str();
			boost::split(unpack, src,
			             boost::is_any_of(","), boost::token_compress_off);
			for ( auto &item : unpack ) {
				boost::trim(item);
			}

			if ( !_unpackRequested ) {
				_unpackRequested = true;
			}
		}

		toks.clear();

		src = streamID.c_str();
		boost::split(toks, src, boost::is_any_of("."), boost::token_compress_off);
		if ( toks.size() != 4 ) {
			LogError("%s:%d: wrong id: expected four items separated by .",
			               streamID.c_str(), line_number);
			return false;
		}

		addRequest(toks[NetworkCode], toks[StationCode], toks[LocationCode],
		           toks[ChannelCode], _startTime, _endTime, unpack, false);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::run() {
	while ( !_exitRequested ) {
		if ( !handshake() ) {
			close();
			return false;
		}

		// Read the data
		while ( !_exitRequested && _socket->isValid() ) {
			// Skip unread bytes from previous record
			int skippies = _socketBuf.read_limit();
			if ( skippies > 0 ) {
				CAPS_WARNING("No seemless reading, skipping %d bytes", skippies);
				if ( _socketBuf.pubseekoff(skippies, ios_base::cur, ios_base::in) < 0 ) {
					LogError("Seek: connection closed");
					break;
				}
			}

			_socketBuf.set_read_limit(-1);
			if ( _exitRequested ) break;

			ResponseHeader responseHeader;
			if ( !_socket->isValid() || !responseHeader.get(_socketBuf) ) {
				CAPS_INFO("Connection closed by peer");
				disconnect();
				wait();
				break;
			}

			CAPS_DEBUG("read %d data bytes", responseHeader.size);
			_socketBuf.set_read_limit(responseHeader.size);

			if ( responseHeader.id == 0 ) {
				// Server ID
				while ( responseHeader.size > 0 ) {
					istream is(&_socketBuf);
					if ( is.getline(_lineBuf, 200).fail() ) {
						LogError("Invalid response: line exceeds maximum of 200 characters");
						// Unrecoverable error, reconnect makes no sense
						_exitRequested = true;
						break;
					}

					SessionTable::Status status =
					    _sessionTable.handleResponse(_lineBuf, is.gcount());
					if ( status == SessionTable::Error ) {
						LogError("Fatal error: invalid response: %s", _lineBuf);
						break;
					} else if ( status == SessionTable::EOD ) {
						CAPS_DEBUG("Got EOD");
						_exitRequested = true;
						break;
					}

					responseHeader.size -= is.gcount();
				}

				if ( responseHeader.size > 0 && !_exitRequested) {
					LogError("Header: connection closed");
					// Reconnect and try to handshake again
					disconnect();
					wait();
					break;
				}
			}
			else {
				if ( _currentID != responseHeader.id ) {
					SessionTableItem *item = _sessionTable.getItem(responseHeader.id);
					if ( !item ) {
						CAPS_WARNING("Unknown data request ID %d", responseHeader.id);
						cerr << "ignoring DATA[] ... " << responseHeader.size << endl;
					}

					_currentItem = item;
					_currentID = responseHeader.id;
				}

				if ( _currentItem ) {
					/*
					  To improve performance CAPS uses an optimized protocol
					  to deliver RAW packets. The RAW packet class can't be used
					  to read the header from socket.
					*/
					if ( _currentItem->type == RawDataPacket ) {
						if ( !_rawResponseHeader.get(_socketBuf) ) {
							LogError("Failed to extract raw response header %s",
							           _currentItem->streamID.c_str());
							continue;
						}

						int size = responseHeader.size;

						Time time(_rawResponseHeader.timeSeconds,
						          _rawResponseHeader.timeMicroSeconds);

						size -= _rawResponseHeader.dataSize();

						RawDataRecord rawRec;

						rawRec.setStartTime(time);
						rawRec.setSamplingFrequency(_currentItem->samplingFrequency,
						                            _currentItem->samplingFrequencyDivider);
						rawRec.setDataType(_currentItem->dataType);
						if ( rawRec.getData(_socketBuf, size) != DataRecord::RS_Complete ) {
							LogError("Failed to read packet data from incoming packet %s",
							           _currentItem->streamID.c_str());
							continue;
						}

						if ( !storeRawPacket(rawRec) ) {
							continue;
						}

						setStreamState(_currentItem->streamID, rawRec.endTime());
						_currentItem->startTime = rawRec.endTime();
					}
					// miniSEED Packets
					else if ( _currentItem->type ==  MSEEDPacket ) {
						MSEEDDataRecord rec;
						DataRecord::ReadStatus status;
						status = rec.get(_socketBuf, responseHeader.size, Time(), Time(), -1);
						if ( status != DataRecord::RS_Complete ) {
							LogError("Failed to read packet data from incoming packet %s",
							           _currentItem->streamID.c_str());
							continue;
						}

						if ( storeMSEEDPacket(rec) ) {
							_currentItem->startTime = rec.endTime();
							setStreamState(_currentItem->streamID, rec.endTime());
						}
					}
					else {
						LogError("Unsupported packet type %s",
						           _currentItem->streamID.c_str());
						continue;
					}
				}
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::setStreamState(const string &streamID, const Time &time) {
	Time now = Time::GMT();

	if ( time > now ) {
		TimeSpan diff = time - now;
		if ( diff > _maximumTimeDiff ) {
			LogWarning("%s: end time offset threshold reached: %s",
			           streamID.c_str(), time.iso().c_str());
			return;
		}
	}

	_states[streamID] = time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::storeRawPacket(RawDataRecord &rec) {
	DataType dt = rec.header()->dataType;
	if ( dt != DT_INT32 ) {
		LogDebug("Unsupported RAW CAPS datatype %d", dt);
		return false;
	}

	int32_t *data = reinterpret_cast<int32_t*>(rec.buffer()->data());
	int32_t dataSize = rec.buffer()->size() / sizeof(int32_t);

	SessionTableItem *item = _currentItem;
	return send_raw_depoch((item->net + "." + item->sta).c_str(), item->cha.c_str(),
	                       rec.startTime(), 0, -1, data, dataSize);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::storeMSEEDPacket(MSEEDDataRecord &rec) {
	SessionTableItem *item = _currentItem;

	if ( _dump ) {
		if ( rec.data()->size() != PLUGIN_MSEED_SIZE ) {
			LogWarning("%s.%s.%s.%s: invalid record size (%d != %d)",
			           item->net.c_str(), item->sta.c_str(),
			           item->loc.c_str(), item->cha.c_str(),
			           int(rec.data()->size()),
			           PLUGIN_MSEED_SIZE);
		}

		cout.write(rec.data()->data(), rec.data()->size());
		cout.flush();
		return true;
	}

	if ( rec.data()->size() != PLUGIN_MSEED_SIZE ) {
		LogWarning("%s.%s.%s.%s: invalid record size (%d != %d)",
		           item->net.c_str(), item->sta.c_str(),
		           item->loc.c_str(), item->cha.c_str(),
		           int(rec.data()->size()),
		           PLUGIN_MSEED_SIZE);
		return true;
	}

	int r = 0;

	if ( item->userData ) {
		SLMSrecord* msr = sl_msr_new();
		sl_msr_parse(nullptr, rec.data()->data(), &msr, 1, 1);

		if ( !msr ) {
			/*
			logs(LOG_ERR) << "error decoding Mini-SEED packet " <<
			  string(fsdh->sequence_number, 6) << ", "
			  "station " << net << "_" << sta << " (" << myid << "), "
			  "stream " << loc << "." << chn << ".D" << endl;
			*/

			return false;
		}

		if ( msr->numsamples < 0 || msr->numsamples > MAX_SAMPLES ) {
			/*
			logs(LOG_ERR) << "error decoding Mini-SEED packet " <<
			  string(fsdh->sequence_number, 6) << ", "
			  "station " << net << "_" << sta << " (" << myid << "), "
			  "stream " << loc << "." << chn << ".D" << endl;
			*/

			sl_msr_free(&msr);
			return false;
		}

		if ( msr->numsamples == 0 ) {
			// not a data record
			sl_msr_free(&msr);
			return false;
		}

		int timing_quality = -1, usec99 = 0;

		if ( msr->Blkt1001 ) {
			timing_quality = msr->Blkt1001->timing_qual;
			usec99 = msr->Blkt1001->usec;
		}

		struct ptime pt;
		rec.startTime().get2(&pt.year, &pt.yday,
		                     &pt.hour, &pt.minute, &pt.second, &pt.usec);

		char id[sizeof(msr->fsdh.network) + 1 + sizeof(msr->fsdh.station) + 1] = {0};
		strncat(id, msr->fsdh.network, sizeof(msr->fsdh.network));
		strcat(id, ".");
		strncat(id, msr->fsdh.station, sizeof(msr->fsdh.station));

		char cha[sizeof(msr->fsdh.location) + sizeof(msr->fsdh.channel) + 1] = {0};
		strncat(cha, msr->fsdh.location, sizeof(msr->fsdh.location));
		strncat(cha, msr->fsdh.channel, sizeof(msr->fsdh.channel));

		r = send_raw3(id, cha, &pt,
		              msr->fsdh.time_correct, timing_quality,
		              msr->datasamples, msr->numsamples);

		sl_msr_free(&msr);
	}
	else {
		r = send_mseed((item->net + "." + item->sta).c_str(), rec.data()->data(), rec.data()->size());
	}

	if ( r <= 0 ) {
		LogError("Link to SeedLink broken: %d: %s", r, strerror(r));
		return false;
	}

	CAPS_DEBUG("%s.%s.%s: published record, next sample time at %s",
	           item->sta.c_str(), item->loc.c_str(), item->cha.c_str(),
	           rec.endTime().toString("%F %T").c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::validateParameters() {
	if( _vm.count("help")) {
		cerr << *_options << endl;
		return false;
	}

	for ( const auto &stream : _streams ) {
		vector<string> toks;
		const char *src = stream.c_str();
		boost::split(toks, src, boost::is_any_of("."), boost::token_compress_off);
		if ( toks.size() != 4 ) {
			LogError("Invalid streamID '%s'", stream.c_str());
			return false;
		}
	}

	if ( !_strStartTime.empty() ) {
		if ( !_startTime.fromString(_strStartTime.c_str(), "%F %T") ) {
			LogError("Wrong start time format, expected 'YEAR-MONTH-DAY HOUR:MINUTE:SECOND'");
			return false;
		}
	}

	if ( !_strEndTime.empty() ) {
		if ( !_endTime.fromString(_strEndTime.c_str(), "%F %T") ) {
			LogError("Wrong end time format, expected 'YEAR-MONTH-DAY HOUR:MINUTE:SECOND'");
			return false;
		}
	}

	if ( !_address.empty() ) {
		if ( !parseURL(_address) ) {
			LogError("Wrong input address format, expected [[caps|capss]://][user:pass@]host[:port]");
			return false;
		}
	}

	if ( _vm.count("dump") ) {
		_dump = true;
	}

	if ( _vm.count("archive") ) {
		_archive = true;
	}

	if ( _vm.count("in-order") ) {
		_allowOutOfOrder = false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::done() {
	disconnect();

	// Free used space
	Application::done();

	fstream ofs;
	ostream *os = &cout;

	if ( !_logFile.empty() ) {
		ofs.open(_logFile.c_str(), ios_base::out | ios_base::ate);
		if ( !ofs.is_open() ) {
			LogError("Unable to save state to %s, write to stdout",
			           _logFile.c_str());
		}
		else {
			os = &ofs;
		}
	}

	CAPS_DEBUG("Flushing states");
	for ( const auto &item : _states )
		(*os) << item.first << " " << item.second.iso() << endl;

	os->flush();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::parseURL(string url) {
	boost::trim(url);

	// step 1: protocol
	string protocol;
	size_t pos = url.find("://");

	if ( pos == string::npos ) {
		protocol = "caps";
	}
	else {
		protocol = url.substr(0, pos);
		url = url.substr(pos + 3);
	}

	_ssl = protocol == "capss";
	if ( !_ssl && protocol != "caps" ) {
		LogError("unsupported protocol: %s", protocol.c_str());
		return false;
	}

	// step 2: user:pass
	vector<string> toks;
	const char *src = url.c_str();
	boost::split(toks, src, boost::is_any_of("@"), boost::token_compress_on);
	if ( toks.size() >= 2 ) {
		string login = toks[0];
		url = toks[1];
		src = url.c_str();
		boost::split(toks, login, boost::is_any_of(":"), boost::token_compress_on);
		_user = toks.size() > 0 ? toks[0] : "";
		_password = toks.size() > 1 ? toks[1] : "";
	}

	// step 3: host:port
	src = url.c_str();

	return splitAddress(_host, _port, src, 18002);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::addRequest(const string &net, const string &sta,
                     const string &loc, const string &cha,
                     const Time &stime, const Time &etime,
                     const Unpack &unpack,
                     bool receivedData) {
	string streamID = net + "." + sta + "." + loc + "." + cha;
	RequestByID::iterator it = _requestByID.find(streamID);
	if ( it == _requestByID.end() ) {
		RequestPtr req(new Request(streamID));
		req->unpack = unpack;
		it = _requestByID.insert(make_pair(streamID, req)).first;
		_requests.push_back(req.get());
	}

	RequestPtr req = it->second;
	req->startTime = stime;
	req->endTime = etime;
	req->receivedData = receivedData;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::close() {
	disconnect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::disconnect() {
	if ( _socket && _socket->isValid() ) {
		_socket->shutdown();
		_socket->close();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool App::handshake() {
	while ( !_socket->isValid() ) {
		for ( const auto &item : _sessionTable ) {
			addRequest(item.second.net, item.second.sta,
			           item.second.loc, item.second.cha,
			           item.second.startTime, item.second.endTime, {}, true);
		}

		_sessionTable.reset();

		bool first = true;
		while ( !_exitRequested ) {
			if ( _socket->connect(_host, _port) != Socket::Success ) {
				if ( first ) {
					CAPS_WARNING("Unable to connect to %s:%d, "
					             "trying again every 5 seconds",
					             _host.c_str(), _port);
					first = false;
				}

				wait();
			}
			else {
				break;
			}
		}

		if ( _exitRequested ) {
			return false;
		}

		// Do handshake
		if ( !_user.empty() && !_password.empty() ) {
			string auth = "AUTH " + _user + " " + _password + "\n";
			_socket->write(auth.c_str(), auth.length());
		}

		_socket->write("BEGIN REQUEST\n", 14);

		if ( _archive )
			_socket->write("REALTIME OFF\n", 13);

		if ( _allowOutOfOrder )
			_socket->write("OUTOFORDER ON\n", 14);
		else
			_socket->write("OUTOFORDER OFF\n", 15);

		// First pass: continue all previous streams
		for ( const auto &item : _requestByID ) {
			if ( item.second->receivedData == false ) {
				continue;
			}

			stringstream req;
			req << "STREAM ADD " << item.first << endl;
			req << "TIME ";

			int year, mon, day, hour, minute, second;

			if ( item.second->startTime.valid() ) {
				item.second->startTime.get(&year, &mon, &day, &hour, &minute, &second);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second;
			}

			req << ":";

			if ( item.second->endTime.valid() ) {
				item.second->endTime.get(&year, &mon, &day, &hour, &minute, &second);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second;
			}

			req << endl;

			string line = req.str();
			CAPS_DEBUG("%s", line.c_str());
			_socket->write(line.c_str(), line.size());
		}

		// Second pass: subscribe to remaining streams
		for ( const auto &item : _requestByID ) {
			if ( item.second->receivedData == true ) {
				continue;
			}

			stringstream req;
			req << "STREAM ADD " << item.first << endl;
			req << "TIME ";

			int year, mon, day, hour, minute, second;

			if ( item.second->startTime.valid() ) {
				item.second->startTime.get(&year, &mon, &day, &hour, &minute, &second);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second;
			}

			req << ":";

			if ( item.second->endTime.valid() ) {
				item.second->endTime.get(&year, &mon, &day, &hour, &minute, &second);
				req << year << "," << mon << "," << day << ","
				    << hour << "," << minute << "," << second;
			}

			req << endl;

			string line = req.str();
			CAPS_DEBUG("%s", line.c_str());
			_socket->write(line.c_str(), line.size());
		}

		_socket->write("END\n", 4);

		_socketBuf.set_read_limit(-1);

		ResponseHeader header;
		if ( !header.get(_socketBuf) ) {
			// Case to retry, connection closed by peer
			LogError("Unable to finish handshake, closing connection and connect again");
			// Do not close, only disconnect to remember last session streams
			disconnect();
			wait();
			continue;
		}

		_socketBuf.set_read_limit(header.size);
		if ( header.id != 0 ) {
			LogError("Invalid handshake response, expected ID 0 but got %d", header.id);
			return false;
		}

		istream is(&_socketBuf);
		if ( is.getline(_lineBuf, 200).fail() ) {
			LogError("Invalid response: line exceeds maximum of 200 characters");
			return false;
		}

		// Skip remaining stuff
		if ( _socketBuf.pubseekoff(_socketBuf.read_limit(), ios_base::cur, ios_base::in) < 0 ) {
			LogError("Seek: connection closed by peer");
			disconnect();
			wait();
			continue;
		}

		if ( strncasecmp(_lineBuf, "ERROR:", 6) == 0 ) {
			// No case to retry
			LogError("Error in handshake, server responds: %s", _lineBuf);
			return false;
		}
		else if ( strncasecmp(_lineBuf, "STATUS OK", 9) == 0 ) {
			CAPS_DEBUG("Handshaking complete");
			return true;
		}
		else {
			// No case to retry
			LogError("Error in handshake, invalid response: %s", _lineBuf);
			return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::itemAdded(SessionTableItem *item) {
	if ( !_unpackRequested ) {
		return;
	}

	for ( const auto &req : _requestByID ) {
		if ( !wildcmp(req.first, item->streamID) ) {
			continue;
		}

		std::string chaID = item->loc + "." + item->cha;
		for ( const auto &expr : req.second->unpack ) {
			if ( wildcmp(expr, chaID) ) {
				// We use the userData attribute to signal that this
				// stream needs to be unpacked before it is forwarded
				// to SeedLink
				item->userData = item;
				break;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::itemAboutToBeRemoved(SessionTableItem *item) {
	if ( _currentItem == item ) {
		_currentItem = nullptr;
		_currentID = -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void App::wait() {
	// Wait 5 seconds and keep response latency low
	for ( int i = 0; (i < 10) && !_exitRequested; ++i )
		usleep(500000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
