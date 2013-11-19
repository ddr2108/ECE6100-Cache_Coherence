#include "MSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MSI_protocol::MSI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    // Initialize lines to not have the data yet!
    this->state = MSI_CACHE_I;
}

MSI_protocol::~MSI_protocol ()
{    
}

void MSI_protocol::dump (void)
{
    const char *block_states[7] = {"X","I","S","M", "IS", "IM", "SM"};
    fprintf (stderr, "MSI_protocol - state: %s\n", block_states[state]);
}

void MSI_protocol::process_cache_request (Mreq *request)
{
	switch (state) {
    case MSI_CACHE_I:  do_cache_I (request); break;
    case MSI_CACHE_S:  do_cache_S (request); break;
    case MSI_CACHE_M:  do_cache_M (request); break;
    case MSI_CACHE_IM: case MSI_CACHE_IS: case MSI_CACHE_SM: break;
    default:
        fatal_error ("Invalid Cache State for MSI Protocol\n");
        break;
    }
}

void MSI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
    case MSI_CACHE_I:  do_snoop_I (request); break;
    case MSI_CACHE_S:  do_snoop_S (request); break;
    case MSI_CACHE_M:  do_snoop_M (request); break;
    case MSI_CACHE_IS:  do_snoop_IS (request); break;
    case MSI_CACHE_IM:  do_snoop_IM (request); break;
    case MSI_CACHE_SM:  do_snoop_SM (request); break;
    default:
    	fatal_error ("Invalid Cache State for MSI Protocol\n");
    }
}

inline void MSI_protocol::do_cache_I (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //Request data
        send_GETS(request->addr);
        state = MSI_CACHE_IS;
        //cache miss
        Sim->cache_misses++;
        break;
    case STORE:
        //Request data to modify
        send_GETM(request->addr);
        state = MSI_CACHE_IM;
        //cache miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_cache_S (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        //ask for data
        send_GETM(request->addr);
        //request data;
        state = MSI_CACHE_SM;
        //Coherence miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_cache_M (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
    case STORE:
        //give processor data
        send_DATA_to_proc(request->addr);
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }

}

inline void MSI_protocol::do_snoop_I (Mreq *request)
{
    //dont need to do anything
    switch (request->msg) {
    case GETS:
    case GETM:
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_S (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        break;
    case GETM:
        //Changed to invalid
        state = MSI_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_M (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        //Send Data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Change to shared
        state = MSI_CACHE_S;
        break;
    case GETM:
        //Send date
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Change to invalid
        state = MSI_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }

}

inline void MSI_protocol::do_snoop_IS (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //get data
        state = MSI_CACHE_S;
        send_DATA_to_proc(request->addr);
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}


inline void MSI_protocol::do_snoop_IM (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //get data
        state = MSI_CACHE_M;
        send_DATA_to_proc(request->addr);
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MSI_protocol::do_snoop_SM (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //Get data
        state = MSI_CACHE_M;
        send_DATA_to_proc(request->addr);
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: I state shouldn't see this message\n");
    }
}
