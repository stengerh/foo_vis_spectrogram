#include "stdafx.h"
#include "visualisation_timer.h"

// {6E1B59A9-B20D-43d7-83CE-D20013DD3057}
const GUID visualisation_timer::class_guid = 
{ 0x6e1b59a9, 0xb20d, 0x43d7, { 0x83, 0xce, 0xd2, 0x0, 0x13, 0xdd, 0x30, 0x57 } };

// {B605AF52-B895-453d-AE76-BE1D2B3ABAA3}
const GUID visualisation_timer_manager::class_guid = 
{ 0xb605af52, 0xb895, 0x453d, { 0xae, 0x76, 0xbe, 0x1d, 0x2b, 0x3a, 0xba, 0xa3 } };

void visualisation_timer_manager::g_create_timer(service_ptr_t<visualisation_timer> & p_timer, unsigned p_elapse)
{
    static_api_ptr_t<visualisation_timer_manager>()->create_timer(p_timer, p_elapse);
}
