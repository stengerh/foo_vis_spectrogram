#ifndef __FOO_VIS_SPECTRUM__SPECTRUM_BUFFER_H__
#define __FOO_VIS_SPECTRUM__SPECTRUM_BUFFER_H__

typedef t_int64 t_spectrum_timestamp;

class spectrum_buffer_metrics
{
private:
    t_spectrum_timestamp m_start_time, m_end_time;
    const t_size m_sample_count, m_size;
    t_size m_offset;

public:
    spectrum_buffer_metrics(t_size p_size, t_size p_sample_count, t_spectrum_timestamp p_start_time = 0);
    spectrum_buffer_metrics(const spectrum_buffer_metrics & p_source, t_size p_new_size);
    spectrum_buffer_metrics(const spectrum_buffer_metrics & p_source, t_spectrum_timestamp p_end_time);

    // range of valid times includes start time, but excludes end time
    t_spectrum_timestamp get_start_time() const {return m_start_time;}
    t_spectrum_timestamp get_end_time() const {return m_end_time;}

    void advance_time(); // advances time by one increment

    //! Get number of samples in each slice.
    t_size get_sample_count() const {return m_sample_count;}

    //! Get number of slices.
    t_size get_size() const {return m_size;}

    //! Get number of used slices.
    t_size get_length() const {return pfc::downcast_guarded<t_size, t_spectrum_timestamp>(get_end_time() - get_start_time());}

    //! Translate timestamp into index to associated buffers.
    // throws exception on out of bounds timestamp
    //! @param p_time timestamp valid range is from get_start_time() to get_end_time(), both inclusive
    //! @returns index corresponding to p_time, do not use value returned for get_end_time() to index associated buffers
    t_size get_index_for_time(t_spectrum_timestamp p_time) const;

    inline t_size get_end_index() const {return get_index_for_time(get_end_time());}
    inline t_size get_start_index() const {return get_index_for_time(get_start_time());}
};

class NOVTABLE spectrum_buffer : public service_base
{
public:
    enum t_scaling_mode
    {
        scaling_mode_gdi_fast,
        scaling_mode_gdi_slow,
        scaling_mode_software_linear,
        scaling_mode_software_cubic,
    };

    virtual void set_size(t_size p_size) = 0;
    virtual t_size get_size() const = 0;

    virtual void set_sample_count(t_size p_sample_count) = 0;
    virtual t_size get_sample_count() const = 0;

    virtual t_size get_length() const = 0;

    virtual t_spectrum_timestamp get_start_time() const = 0;
    virtual t_spectrum_timestamp get_end_time() const = 0;

    virtual t_size get_index_for_time(t_spectrum_timestamp p_time) const = 0;

    virtual t_spectrum_timestamp add_chunk(const audio_chunk & p_chunk) = 0;

    virtual void set_colormap(const service_ptr_t<colormap> & p_colormap) = 0;

    virtual void draw(HDC hdc, CRect rcTarget, t_spectrum_timestamp p_base_time, COLORREF clrBackground, t_scaling_mode p_quality) const = 0;

    inline t_size get_start_index() const {return get_index_for_time(get_start_time());}
    inline t_size get_end_index() const {return get_index_for_time(get_end_time());}

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(spectrum_buffer);
};

#endif
