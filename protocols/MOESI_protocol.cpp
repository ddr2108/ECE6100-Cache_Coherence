#include "MOESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESI_protocol::MOESI_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    // Initialize lines to not have the data yet!
    this->state = MOESI_CACHE_I;
    isOwner = -1;
}

MOESI_protocol::~MOESI_protocol ()
{    
}

void MOESI_protocol::dump (void)
{
    const char *block_states[9] = {"X","I","S","E","O","M", "IS", "IM", "SM"};
    fprintf (stderr, "MOESI_protocol - state: %s\n", block_states[state]);
}

void MOESI_protocol::process_cache_request (Mreq *request)
{
    switch (state) {
    case MOESI_CACHE_I:  do_cache_I (request); break;
    case MOESI_CACHE_S:  do_cache_S (request); break;
    case MOESI_CACHE_E:  do_cache_E (request); break;    
    case MOESI_CACHE_O:  do_cache_O (request); break;
    case MOESI_CACHE_M:  do_cache_M (request); break;
    case MOESI_CACHE_IM: case MOESI_CACHE_IS: case MOESI_CACHE_SM: break;          //Should not be getting requests
    default:
        fatal_error ("Invalid Cache State for MOESI Protocol\n");
        break;
    }
}

void MOESI_protocol::process_snoop_request (Mreq *request)
{
	switch (state) {
    case MOESI_CACHE_I:  do_snoop_I (request); break;
    case MOESI_CACHE_S:  do_snoop_S (request); break;
    case MOESI_CACHE_E:  do_snoop_E (request); break;
    case MOESI_CACHE_O:  do_snoop_O (request); break;
    case MOESI_CACHE_M:  do_snoop_M (request); break;
    case MOESI_CACHE_IS:  do_snoop_IS (request); break;
    case MOESI_CACHE_IM:  do_snoop_IM (request); break;
    case MOESI_CACHE_SM:  do_snoop_SM (request); break;
    default:
        fatal_error ("Invalid Cache State for MOESI Protocol\n");
    }
}

inline void MOESI_protocol::do_cache_I (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //Request data
        send_GETS(request->addr);
        state = MOESI_CACHE_IS;
        //cache miss
        Sim->cache_misses++;
        break;
    case STORE:
        //Request data to modify
        send_GETM(request->addr);
        state = MOESI_CACHE_IM;
        //cache miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_cache_S (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        //ask for data
        send_GETM(request->addr);
        state = MOESI_CACHE_SM;
        //Coherence Miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_cache_E (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        //send data to processor
        send_DATA_to_proc(request->addr);
        state = MOESI_CACHE_M;
        //Silent upgrade
        Sim->silent_upgrades++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_cache_O (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        isOwner =request->src_mid.nodeID;    //Set flag saying it was owner
        //ask for data
        send_GETM(request->addr);
        state = MOESI_CACHE_SM;
        //Coherence Miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_cache_M (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
    case STORE:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_I (Mreq *request)
{
    //dont need to do anything
    switch (request->msg) {
    case GETS:
    case GETM:
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_S (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        //So that IM will know whether to make exclusive
        set_shared_line();
        break;
    case GETM:
        //Changed to invalid
        state = MOESI_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_E (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        //Send Data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Change to shared
        state = MOESI_CACHE_S;
        break;
    case GETM:
        //Send Data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Change to invalid
        state = MOESI_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_O (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        break;
    case GETM:
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Changed to invalid state
        state = MOESI_CACHE_I;
        break;
    case DATA: 
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESI_protocol::do_snoop_M (Mreq *request)
{
    switch (request->msg) {
    case GETS:
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //now owner of modified data
        state = MOESI_CACHE_O;
        break;
    case GETM:
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //now invalidated
        state = MOESI_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESI_protocol::do_snoop_IS (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //send data
        send_DATA_to_proc(request->addr);
        //Determine state based on whether it came from a shared state
        if (get_shared_line()){
            state = MOESI_CACHE_S;
        } else{
            state = MOESI_CACHE_E;            
        }
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESI_protocol::do_snoop_IM (Mreq *request)
{
    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //get data and set to modified
        send_DATA_to_proc(request->addr);
        //Change to modify state
        state = MOESI_CACHE_M;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESI_protocol::do_snoop_SM (Mreq *request)
{
    switch (request->msg) {
     case GETS:
        set_shared_line();
        
        if(isOwner!=-1 && !get_shared_line()){
            //give data
            send_DATA_on_bus(request->addr,request->src_mid);
        }
        break;
    case GETM:

        if(isOwner!=-1 && !get_shared_line()){
            //give data
            set_shared_line();
            send_DATA_on_bus(request->addr,request->src_mid);
        }
        if (request->src_mid.nodeID != isOwner){
             //Change state
            state = MOESI_CACHE_IM;
            //Reset flag
            isOwner = -1;
        }

        break;
    case DATA:
        //Reset flag
        isOwner = -1;

        //Get data
        send_DATA_to_proc(request->addr);
        
        //Change to modify state
        state = MOESI_CACHE_M;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

