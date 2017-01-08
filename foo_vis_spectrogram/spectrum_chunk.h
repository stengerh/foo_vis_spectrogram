#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_CHUNK_H__
#define __FOO_VIS_SPECTRUM__SPECTRUM_CHUNK_H__

class NOVTABLE spectrum_chunk
{
public:
	virtual COLORREF * get_data()=0;
	virtual const COLORREF * get_data() const = 0;

	virtual t_size get_data_size() const = 0;
	virtual void set_data_size(t_size p_new_size)=0;

	virtual unsigned get_sample_rate() const = 0;
	virtual void set_sample_rate(unsigned val)=0;

	virtual unsigned get_channels() const = 0;
	virtual unsigned get_channel_config() const = 0;
	virtual void set_channels(unsigned p_count, unsigned p_config)=0;

	virtual t_size get_sample_count() const = 0;
	virtual void set_sample_count(t_size val)=0;

	void set_data(const COLORREF * p_source, t_size p_sample_count, unsigned p_channel_count, unsigned p_sample_rate, unsigned p_channel_config);
	void set_data(const audio_chunk & p_source, service_ptr_t<colormap> p_mapper);

	void set_channels(unsigned val);

	//! Returns actual amount of audio data contained in the buffer (sample count * channel count). Must not be greater than data size (see get_data_size()).
	inline t_size get_data_length() const {return get_sample_count() * get_channels();}

	//! Resets all spectrum_chunk data.
	inline void reset() {
		set_sample_count(0);
		set_sample_rate(0);
		set_channels(0);
		set_data_size(0);
	}

	void copy(const spectrum_chunk & p_source) {
		set_data(
			p_source.get_data(),
			p_source.get_sample_count(),
			p_source.get_channels(),
			p_source.get_sample_rate(),
			p_source.get_channel_config());
	}

	void draw(HDC hdc, int x, int y, int height, const pfc::list_base_const_t<unsigned> & p_channels);

	void draw_single(HDC hdc, int x, int y, int height, unsigned p_channel) {return draw(hdc, x, y, height, pfc::list_single_ref_t<unsigned>(p_channel));}
	void draw_auto(HDC hdc, int x, int y, int height);

protected:
	spectrum_chunk() {}
	~spectrum_chunk() {}
};

class spectrum_chunk_impl : public spectrum_chunk
{
public:
	spectrum_chunk_impl() : m_sample_rate(0), m_channel_count(0), m_sample_count(0), m_channel_config(0) {}
	
	virtual COLORREF * get_data() {return m_data.get_ptr();}
	virtual const COLORREF * get_data() const {return m_data.get_ptr();}

	virtual t_size get_data_size() const {return m_data.get_size();}
	virtual void set_data_size(t_size p_new_size) {m_data.set_size(p_new_size);}

	virtual unsigned get_sample_rate() const {return m_sample_rate;}
	virtual void set_sample_rate(unsigned val) {m_sample_rate = val;}

	virtual unsigned get_channels() const {return m_channel_count;}
	virtual unsigned get_channel_config() const {return m_channel_config;}
	virtual void set_channels(unsigned p_count, unsigned p_config) {m_channel_count = p_count; m_channel_config = p_config;}

	virtual t_size get_sample_count() const {return m_sample_count;}
	virtual void set_sample_count(t_size val) {m_sample_count = val;}

private:
	pfc::array_t<COLORREF> m_data;
	unsigned m_sample_rate, m_channel_count, m_channel_config;
	t_size m_sample_count;
};

#endif
