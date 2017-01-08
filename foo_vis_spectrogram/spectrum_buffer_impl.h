#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_BUFFER_IMPL_H__
#define __FOO_VIS_SPECTRUM__SPECTRUM_BUFFER_IMPL_H__

template <typename T>
class subarray_accessor_t
{
private:
	T * m_data;
	t_size m_width;

public:
	subarray_accessor_t(T * p_data, t_size p_width) : m_data(p_data), m_width(p_width) {}
	subarray_accessor_t(const subarray_accessor_t<T> & p_source) : m_data(p_source.m_data), m_width(p_source.m_width) {}

	inline T & operator [](t_size p_index) {return m_data[p_index * m_width];}
	inline const T & operator [](t_size p_index) const {return m_data[p_index * m_width];}
};

template <typename T>
class subarray_accessor_const_t
{
private:
	const T * m_data;
	t_size m_width;

public:
	subarray_accessor_const_t(const T * p_data, t_size p_width) : m_data(p_data), m_width(p_width) {}
	subarray_accessor_const_t(const subarray_accessor_const_t<T> & p_source) : m_data(p_source.m_data), m_width(p_source.m_width) {}

	inline const T & operator [](t_size p_index) const {return m_data[p_index * m_width];}
};

template <typename T, typename t_accessor>
class accessor_repeat_borders_t
{
private:
	t_accessor m_accessor;
	int m_size;

public:
	accessor_repeat_borders_t(t_accessor p_accessor, int p_size) : m_accessor(p_accessor), m_size(p_size) {}

	inline const T & operator [](int p_index) const
	{
		if (p_index < 0) p_index = 0;
		else if (p_index >= m_size) p_index = m_size - 1;
		return m_accessor[p_index];
	}
};

//! Holds one track of the spectrum buffer.
//! Actual track types implemented by subclasses.
class spectrum_partial_buffer_base
{
private:
	pfc::rcptr_t<spectrum_buffer_metrics> m_metrics;

protected:
	spectrum_partial_buffer_base(pfc::rcptr_t<spectrum_buffer_metrics> p_metrics) : m_metrics(p_metrics) {}

public:
	inline t_size get_sample_count() const {return m_metrics->get_sample_count();}
	inline t_size get_size() const {return m_metrics->get_size();}
	inline t_size get_length() const {return m_metrics->get_length();}

	inline t_spectrum_timestamp get_start_time() const {return m_metrics->get_start_time();}
	inline t_spectrum_timestamp get_end_time() const {return m_metrics->get_end_time();}

	inline t_size get_index_for_time(t_spectrum_timestamp p_time) const {return m_metrics->get_index_for_time(p_time);}
	inline t_size get_start_index() const {return m_metrics->get_start_index();}
	inline t_size get_end_index() const {return m_metrics->get_end_index();}
};

template <typename T>
class spectrum_channel_buffer_t : public spectrum_partial_buffer_base
{
private:
	pfc::array_t<T> m_data;

public:
	spectrum_channel_buffer_t(pfc::rcptr_t<spectrum_buffer_metrics> p_metrics) : spectrum_partial_buffer_base(p_metrics)
	{
		m_data.set_size(get_size() * get_sample_count());
	}

	spectrum_channel_buffer_t(pfc::rcptr_t<spectrum_buffer_metrics> p_metrics, const spectrum_channel_buffer_t<T> & p_source) : spectrum_partial_buffer_base(p_metrics)
	{
		PFC_ASSERT(get_sample_count() == p_source.get_sample_count());
		PFC_ASSERT(get_length() <= p_source.get_length());
		PFC_ASSERT(get_end_time() == p_source.get_end_time());

		m_data.set_size(get_size() * get_sample_count());
		t_size sample_count = get_sample_count();
		for (t_spectrum_timestamp time = get_start_time(); time < get_end_time(); time++)
		{
			subarray_accessor_const_t<T> from = p_source.get_items_for_time(time);
			subarray_accessor_t<T> to = get_items_for_time(time);
			t_size from_index = p_source.get_index_for_time(time);
			t_size to_index = get_index_for_time(time);
			for (t_size n = 0; n < sample_count; n++)
				to[n] = from[n];
		}
	}

	const T & get_item(t_size p_index, t_size p_sample) const {return m_data[p_index + p_sample * get_size()];}
	void set_item(t_size p_index, t_size p_sample, const T & p_value) {m_data[p_index + p_sample * get_size()] = p_value;}

	inline const T * get_item_for_time(t_spectrum_timestamp p_time, t_size p_sample) const {return get_data(get_index_for_time(p_time));}

	const T * get_ptr() {return m_data.get_ptr();}

	subarray_accessor_const_t<T> get_items(t_size p_index) const {return subarray_accessor_const_t<T>(&m_data[p_index], get_size());}
	subarray_accessor_t<T> get_items(t_size p_index) {return subarray_accessor_t<T>(&m_data[p_index], get_size());}

	inline subarray_accessor_const_t<T> get_items_for_time(t_spectrum_timestamp p_time) const {return get_items(get_index_for_time(p_time));}
	inline subarray_accessor_t<T> get_items_for_time(t_spectrum_timestamp p_time) {return get_items(get_index_for_time(p_time));}
};

template <typename t_source_accessor, typename t_target_accessor>
void interpolate_nearest_neighbour_t(t_source_accessor p_source, t_size p_source_size, t_target_accessor p_target, t_size p_target_size)
{
	PFC_ASSERT(p_source_size > 0);

	if (p_target_size > 1)
	{
		for (t_size n = 0; n < p_target_size; n++)
		{
			t_size m = MulDiv(n, p_source_size - 1, p_target_size - 1);
			p_target[n] = p_source[m];
		}
	}
	else if (p_target_size == 1)
	{
		p_target[0] = p_source[0];
	}
}

template <typename t_source_accessor, typename t_target_accessor>
void interpolate_linear_t(t_source_accessor p_source, int p_source_size, t_target_accessor p_target, int p_target_size)
{
	PFC_ASSERT(p_source_size > 0);

	if (p_target_size > 1)
	{
		for (int n = 0; n < p_target_size; n++)
		{
			int y = MulDiv(n, (p_source_size - 1) * 256, (p_target_size - 1));
			int y0 = y / 256;
			int dy = y & 255;
			p_target[n] = (p_source[y0] * (256 - dy) + p_source[y0 + 1] * dy) / 256;
		}
	}
	else if (p_target_size == 1)
	{
		p_target[0] = p_source[0];
	}
}

template <typename T>
inline T cubic_t(T x) {return x*x*x;}

inline int cubic_fixpoint(int x)
{
	return (x * x * x) >> 16;
}

inline int P(int x) {return (x > 0) ? x : 0;}

inline int R(int x)
{
	return (
		      cubic_t(P(x + 0x200))
		- 4 * cubic_t(P(x + 0x100))
		+ 6 * cubic_t(P(x))
		- 4 * cubic_t(P(x - 0x100))) / 0x60000;
}

template <typename t_source_accessor, typename t_target_accessor>
void interpolate_cubic_t(t_source_accessor p_source, int p_source_size, t_target_accessor p_target, int p_target_size)
{
	PFC_ASSERT(p_source_size > 0);

	if (p_target_size > 1)
	{
		for (int n = 0; n < p_target_size; n++)
		{
			int y = MulDiv(n, (p_source_size - 1) * 256, (p_target_size - 1));
			int y0 = y / 256;
			int dy = y & 0xff;
			int Rm1 = R(-0x100 - dy);
			int R0  = R( 0x000 - dy);
			int R1  = R( 0x100 - dy);
			int R2  = R( 0x200 - dy);
			p_target[n] = (p_source[y0 - 1] * Rm1 + p_source[y0] * R0 + p_source[y0 + 1] * R1 + p_source[y0 + 2] * R2) / 256;
		}
	}
	else if (p_target_size == 1)
	{
		p_target[0] = p_source[0];
	}
}

template <typename T>
class spectrum_track_buffer_t : public spectrum_partial_buffer_base
{
private:
	pfc::array_t<T> m_data;

public:
	spectrum_track_buffer_t(pfc::rcptr_t<spectrum_buffer_metrics> p_metrics) : spectrum_partial_buffer_base(p_metrics)
	{
		m_data.set_size(get_size());
	}

	spectrum_track_buffer_t(pfc::rcptr_t<spectrum_buffer_metrics> p_metrics, const spectrum_track_buffer_t<T> p_source) : spectrum_partial_buffer_base(p_metrics)
	{
		PFC_ASSERT(get_length() <= p_source.get_length());
		PFC_ASSERT(get_end_time() == p_source.get_end_time());

		m_data.set_size(get_size());
		for (t_spectrum_timestamp time = get_start_time(); time < get_end_time(); time++)
		{
			get_data_for_time(time) = p_source.get_data_for_time(time);
		}
	}

	T & get_data(t_size p_index) {return m_data[p_index];}
	const T & get_data(t_size p_index) const {return m_data[p_index];}

	inline T & get_data_for_time(t_spectrum_timestamp p_time) {return get_data(get_index_for_time(p_time));}
	inline const T & get_data_for_time(t_spectrum_timestamp p_time) const {return get_data(get_index_for_time(p_time));}
};

class spectrum_buffer_impl : public spectrum_buffer
{
public:
	spectrum_buffer_impl();

	virtual void set_size(t_size p_size);
	virtual t_size get_size() const {return m_metrics->get_size();}

	virtual void set_sample_count(t_size p_sample_count);
	virtual t_size get_sample_count() const {return m_metrics->get_sample_count();}

	virtual t_size get_length() const {return m_metrics->get_length();}

	virtual t_spectrum_timestamp get_start_time() const {return m_metrics->get_start_time();}
	virtual t_spectrum_timestamp get_end_time() const {return m_metrics->get_end_time();}

	virtual t_size get_index_for_time(t_spectrum_timestamp p_time) const {return m_metrics->get_index_for_time(p_time);}

	virtual t_spectrum_timestamp add_chunk(const audio_chunk & p_chunk);

	virtual void set_colormap(const service_ptr_t<colormap> & p_colormap);

	virtual void draw(HDC hdc, CRect rcTarget, t_spectrum_timestamp p_base_time, COLORREF clrBackground, t_scaling_mode p_quality) const;

private:
	struct t_aux_info
	{
		unsigned m_channel_count;
		unsigned m_channel_config;

		t_aux_info() : m_channel_count(0), m_channel_config(0) {}
		t_aux_info(unsigned p_channel_count, unsigned p_channel_config) : m_channel_count(p_channel_count), m_channel_config(p_channel_config) {}
	};

	typedef t_uint8 spectrum_sample;
	typedef spectrum_channel_buffer_t< spectrum_sample > spectrum_channel_buffer;

	pfc::rcptr_t< spectrum_buffer_metrics > m_metrics;
	pfc::array_t< pfc::rcptr_t<spectrum_channel_buffer> > m_channel;
	pfc::rcptr_t< spectrum_track_buffer_t< t_aux_info > > m_aux_info;

	void set_channels(t_size p_index, unsigned p_channel_count, unsigned p_channel_config);

	pfc::array_t<long> m_channel_refcount;

	long channel_addref(unsigned p_channel);
	long channel_release(unsigned p_channel);

	struct BITMAPINFOSPECTRUM
	{
		BITMAPINFOHEADER bmiHeader;
		RGBQUAD          bmiColors[256];

		//LPBITMAPINFO operator &() {return reinterpret_cast<LPBITMAPINFO>(this);}
		const LPBITMAPINFO operator &() const {return (const LPBITMAPINFO)this;}
	};

	BITMAPINFOSPECTRUM m_bmi;

	void update_bitmap_info();

	void draw_channels(CDCHandle dc, CRect rcDst, CRect rcSrc, t_scaling_mode p_quality) const;
	void draw_channels_uniform(CDCHandle dc, CRect rcDst, CRect rcSrc, t_scaling_mode p_quality) const;
	void draw_channel(CDCHandle dc, CRect rcDst, CRect rcSrc, t_size p_channel, t_scaling_mode p_quality) const;
};

#endif
