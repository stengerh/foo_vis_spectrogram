#ifndef __FOO_VIS_SPECTRUM__VISUALISATION_TIMER_H__
#define __FOO_VIS_SPECTRUM__VISUALISATION_TIMER_H__

class NOVTABLE visualisation_timer_callback
{
public:
    virtual void on_visualisation_timer() = 0;
};

class NOVTABLE visualisation_timer : public service_base
{
public:
    virtual void add_callback(visualisation_timer_callback * p_callback) = 0;
    virtual void remove_callback(visualisation_timer_callback * p_callback) = 0;

    FB2K_MAKE_SERVICE_INTERFACE(visualisation_timer, service_base);
};

//! Manages synchronized timers.
//! If two requests for timer objects with the same time-out are made,
//! the system guarantees that the callbacks for those timers are
//! called at the same time, but in unspecified order.
//! @pre Timers may only be created in the main thread.
class NOVTABLE visualisation_timer_manager : public service_base
{
public:
    //! Create timer with given time-out.
    //! @param [out] p_timer created timer will be stored in here
    //! @param [in] p_elapse time-out value in milliseconds
    virtual void create_timer(service_ptr_t<visualisation_timer> & p_timer, unsigned p_elapse) = 0;

    //! Create timer with given frequency.
    //! @param [out] p_timer created timer will be stored in here
    //! @param [in] p_rate frequency in Hertz
    inline void create_timer_for_rate(service_ptr_t<visualisation_timer> & p_timer, unsigned p_rate) {create_timer(p_timer, 1000 / pfc::clip_t<unsigned>(p_rate, 1, 100));}

    //! Create timer with given time-out.
    //! @param [out] p_timer created timer will be stored in here
    //! @param [in] p_elapse time-out value in milliseconds
    static void g_create_timer(service_ptr_t<visualisation_timer> & p_timer, unsigned p_elapse);

    //! Create timer with given frequency.
    //! @param [out] p_timer created timer will be stored in here
    //! @param [in] p_rate frequency in Hertz
    inline static void g_create_timer_for_rate(service_ptr_t<visualisation_timer> & p_timer, unsigned p_rate) {g_create_timer(p_timer, 1000 / pfc::clip_t<unsigned>(p_rate, 1, 100));}

    FB2K_MAKE_SERVICE_INTERFACE_ENTRYPOINT(visualisation_timer_manager);
};

#endif
