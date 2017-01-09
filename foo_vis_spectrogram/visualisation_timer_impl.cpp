#include "stdafx.h"
#include "visualisation_timer.h"

class NOVTABLE visualisation_timer_owner
{
public:
    virtual void add_callback(unsigned p_elapse, visualisation_timer_callback * p_callback) = 0;
    virtual void remove_callback(unsigned p_elapse, visualisation_timer_callback * p_callback) = 0;
};

class visualisation_timer_impl : public visualisation_timer
{
private:
    visualisation_timer_owner * m_owner;
    unsigned m_elapse;
    pfc::ptr_list_t<visualisation_timer_callback> m_callbacks;

public:
    visualisation_timer_impl(visualisation_timer_owner * p_owner, unsigned p_elapse);
    ~visualisation_timer_impl();

    void add_callback(visualisation_timer_callback *p_callback);
    void remove_callback(visualisation_timer_callback *p_callback);
};

class visualisation_timer_manager_impl : public visualisation_timer_manager, private visualisation_timer_owner
{
private:
    struct t_entry
    {
        unsigned m_elapse;
        UINT_PTR m_event_id;
        pfc::ptr_list_t<visualisation_timer_callback> m_callbacks;

        t_entry(unsigned p_elapse) : m_elapse(p_elapse), m_event_id(0) {}
    };
    pfc::list_t< pfc::rcptr_t< t_entry > > m_data;

public:
    void create_timer(service_ptr_t<visualisation_timer> & p_timer, unsigned p_elapse);
    void add_callback(unsigned p_elapse, visualisation_timer_callback * p_callback);
    void remove_callback(unsigned p_elapse, visualisation_timer_callback * p_callback);

private:
    bool find_entry(unsigned p_elapse, t_size & p_index) const;

    static VOID CALLBACK TimerProc(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime);
    void fire_timer(UINT_PTR p_event_id);
};





visualisation_timer_impl::visualisation_timer_impl(visualisation_timer_owner * p_owner, unsigned p_elapse)
    : m_owner(p_owner), m_elapse(p_elapse)
{
}

visualisation_timer_impl::~visualisation_timer_impl()
{
    for (t_size n = 0; n < m_callbacks.get_count(); n++)
        m_owner->remove_callback(m_elapse, m_callbacks[n]);
}

void visualisation_timer_impl::add_callback(visualisation_timer_callback *p_callback)
{
    core_api::ensure_main_thread();

    if (!m_callbacks.have_item(p_callback))
    {
        m_owner->add_callback(m_elapse, p_callback);
        m_callbacks.add_item(p_callback);
    }
}

void visualisation_timer_impl::remove_callback(visualisation_timer_callback *p_callback)
{
    core_api::ensure_main_thread();

    if (m_callbacks.have_item(p_callback))
    {
        m_owner->remove_callback(m_elapse, p_callback);
        m_callbacks.remove_item(p_callback);
    }
}

void visualisation_timer_manager_impl::create_timer(service_ptr_t<visualisation_timer> & p_timer, unsigned p_elapse)
{
    core_api::ensure_main_thread();

    p_timer = new service_impl_t<visualisation_timer_impl>(static_cast<visualisation_timer_owner *>(this), p_elapse);
}

void visualisation_timer_manager_impl::add_callback(unsigned p_elapse, visualisation_timer_callback * p_callback)
{
    t_size index;
    if (!find_entry(p_elapse, index))
    {
        index = m_data.add_item(pfc::rcnew_t< t_entry >(p_elapse));
    }
    if (m_data[index]->m_event_id == 0)
    {
        m_data[index]->m_event_id = ::SetTimer(NULL, p_elapse, p_elapse, &TimerProc);
    }
    m_data[index]->m_callbacks.add_item(p_callback);
}

void visualisation_timer_manager_impl::remove_callback(unsigned p_elapse, visualisation_timer_callback * p_callback)
{
    t_size index;
    if (find_entry(p_elapse, index))
    {
        m_data[index]->m_callbacks.remove_item(p_callback);
        if (m_data[index]->m_callbacks.get_count() == 0)
        {
            ::KillTimer(NULL, m_data[index]->m_event_id);
            m_data.remove_by_idx(index);
        }
    }
}

bool visualisation_timer_manager_impl::find_entry(unsigned p_elapse, t_size & p_index) const
{
    for (t_size n = 0; n < m_data.get_count(); n++)
    {
        if (p_elapse == m_data[n]->m_elapse)
        {
            p_index = n;
            return true;
        }
    }
    return false;
}

void visualisation_timer_manager_impl::fire_timer(UINT_PTR p_event_id)
{
    for (t_size index = 0; index < m_data.get_count(); index++)
    {
        pfc::rcptr_t< t_entry > entry = m_data[index];
        if (entry->m_event_id == p_event_id)
        {
            for (t_size n = 0; n < entry->m_callbacks.get_count(); n++)
            {
                entry->m_callbacks[n]->on_visualisation_timer();
            }
            break;
        }
    }
}

static service_factory_single_transparent_t<visualisation_timer_manager_impl> foo_timer_manager;

VOID CALLBACK visualisation_timer_manager_impl::TimerProc(HWND hWnd, UINT msg, UINT_PTR idEvent, DWORD dwTime)
{
    foo_timer_manager.fire_timer(idEvent);
}
