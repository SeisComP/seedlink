/***************************************************************************
 * (C) 2021 Helmholtz-Zentrum Potsdam - Deutsches GeoForschungsZentrum GFZ *
 * All rights reserved.                                                    *
 *                                                                         *
 * This file may be used under the terms of the GNU Affero                 *
 * Public License version 3.0 as published by the Free Software Foundation *
 * and appearing in the file LICENSE included in the packaging of this     *
 * file. Please review the following information to ensure the GNU Affero  *
 * Public License version 3.0 requirements will be met:                    *
 * https://www.gnu.org/licenses/agpl-3.0.html.                             *
 ***************************************************************************/

template<typename T>
Channel<T>::Channel(const std::string &networkCode,
		    const std::string &stationCode,
		    const std::string &locationCode,
		    const std::string &channelCode,
		    double sampleRate)
: _stationId(networkCode + "." + stationCode) {
	_msr = msr3_init(NULL);

	 if ( ms_nslc2sid(_msr->sid, LM_SIDLEN, 0, networkCode.c_str(),
						   stationCode.c_str(),
						   locationCode.c_str(),
						   channelCode.c_str()) < 0 )
		throw std::logic_error("could not create source identifier");

	_msr->formatversion = 2;
	_msr->reclen = 512;
	_msr->samprate = sampleRate;

	if ( boost::is_same<T, std::int32_t>::value ) {
		_msr->sampletype = 'i';
		_msr->encoding = DE_STEIM2;
	}
	else if ( boost::is_same<T, float>::value ) {
		_msr->sampletype = 'f';
		_msr->encoding = DE_FLOAT32;
	}
	else if ( boost::is_same<T, double>::value ) {
		_msr->sampletype = 'd';
		_msr->encoding = DE_FLOAT64;
	}
	else {
		throw std::logic_error("unsupported data type");
	}
}


template<typename T>
Channel<T>::~Channel() {
	flush(true);
	msr3_free(&_msr);
}


template<typename T>
bool Channel<T>::flush(bool force) {
	int64_t packedsamples = 0;
	_msr->datasamples = (void *)_data.data();
	_msr->numsamples = _data.size();
	_msr->starttime = MS_EPOCH2NSTIME(double(_time));

	msr3_pack(_msr, [](char *record, int reclen, void *handlerdata) {
		Channel* obj = reinterpret_cast<Channel *>(handlerdata);
		obj->send(record, reclen);
	}, this, &packedsamples, force, false);

	_msr->datasamples = NULL;
	_data.erase(_data.begin(), _data.begin() + packedsamples);

	if ( force && _data.size() > 0 )
		throw std::logic_error("remaining data after forced flush");

	return (packedsamples > 0);
}


template<typename T>
void Channel<T>::put(Core::Time t, T value) {
	if ( _data.size() > 0 && t != _time + Core::TimeSpan(double(_data.size()) / _msr->samprate) )
		flush(true);

	_time = t - Core::TimeSpan(double(_data.size()) / _msr->samprate);

	_data.push_back(value);

	if ( flush(false) && _data.size() > 0 )
		_time = t - Core::TimeSpan(double(_data.size() - 1) / _msr->samprate);
}


template<typename T>
void Channel<T>::send(const char *record, size_t len) {
	if ( send_mseed(_stationId.c_str(), record, len) < 0 )
		throw std::system_error(errno,
					std::generic_category(),
					"error sending data to SeedLink server");
}
