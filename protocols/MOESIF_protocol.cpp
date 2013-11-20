#include "MOESIF_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOESIF_protocol::MOESIF_protocol (Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry)
{
    // Initialize lines to not have the data yet
    this->state = MOESIF_CACHE_I;
    isOwner = 0;
    isForward = 0;
}

MOESIF_protocol::~MOESIF_protocol ()
{    
}

void MOESIF_protocol::dump (void)
{
    const char *block_states[10] = {"X","I","S","E","O","M","F", "IS", "IM", "SM"};
    fprintf (stderr, "MOESIF_protocol - state: %s\n", block_states[state]);
}

void MOESIF_protocol::process_cache_request (Mreq *request)
{
    switch (state) {
    case MOESIF_CACHE_F:  do_cache_F (request); break;
    case MOESIF_CACHE_I:  do_cache_I (request); break;
    case MOESIF_CACHE_S:  do_cache_S (request); break;
    case MOESIF_CACHE_E:  do_cache_E (request); break;    
    case MOESIF_CACHE_O:  do_cache_O (request); break;
    case MOESIF_CACHE_M:  do_cache_M (request); break;
    case MOESIF_CACHE_IM: case MOESIF_CACHE_IS: case MOESIF_CACHE_SM: break;          //Should not be getting requests
    default:
        fatal_error ("Invalid Cache State for MOESIF Protocol\n");
        break;
    }
}

void MOESIF_protocol::process_snoop_request (Mreq *request)
{
    switch (state) {
    case MOESIF_CACHE_F:  do_snoop_F (request); break;
    case MOESIF_CACHE_I:  do_snoop_I (request); break;
    case MOESIF_CACHE_S:  do_snoop_S (request); break;
    case MOESIF_CACHE_E:  do_snoop_E (request); break;
    case MOESIF_CACHE_O:  do_snoop_O (request); break;
    case MOESIF_CACHE_M:  do_snoop_M (request); break;
    case MOESIF_CACHE_IS:  do_snoop_IS (request); break;
    case MOESIF_CACHE_IM:  do_snoop_IM (request); break;
    case MOESIF_CACHE_SM:  do_snoop_SM (request); break;
    default:
        fatal_error ("Invalid Cache State for MOESIF Protocol\n");
    }
}

inline void MOESIF_protocol::do_cache_F (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        //ask for data
        send_GETM(request->addr);
        state = MOESIF_CACHE_SM;
        //Coherence Miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_I (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //Request data
        send_GETS(request->addr);
        state = MOESIF_CACHE_IS;
        //cache miss
        Sim->cache_misses++;
        break;
    case STORE:
        //Request data to modify
        send_GETM(request->addr);
        state = MOESIF_CACHE_IM;
        //cache miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_cache_S (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        //ask for data
        send_GETM(request->addr);
        state = MOESIF_CACHE_SM;
        //Coherence Miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_cache_E (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        //send data to processor
        send_DATA_to_proc(request->addr);
        state = MOESIF_CACHE_M;
        //Silent upgrade
        Sim->silent_upgrades++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_cache_O (Mreq *request)
{
    switch (request->msg) {
    case LOAD:
        //send data to processor
        send_DATA_to_proc(request->addr);
        break;
    case STORE:
        isOwner =1;    //Set flag saying it was owner
        //ask for data
        send_GETM(request->addr);
        state = MOESIF_CACHE_SM;
        //Coherence Miss
        Sim->cache_misses++;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_cache_M (Mreq *request)
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

inline void MOESIF_protocol::do_snoop_F (Mreq *request)
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
        //Changed to invalid
        state = MOESIF_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_snoop_I (Mreq *request)
{
    fprintf(stderr, "1\n");
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

inline void MOESIF_protocol::do_snoop_S (Mreq *request)
{
        fprintf(stderr, "2\n");

    switch (request->msg) {
    case GETS:
        //So that IM will know whether to make exclusive
        set_shared_line();
        break;
    case GETM:
        //Changed to invalid
        state = MOESIF_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_snoop_E (Mreq *request)
{
        fprintf(stderr, "3\n");

    switch (request->msg) {
    case GETS:
        //Send Data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Change to forwarding
        state = MOESIF_CACHE_F;
        isForward = 1;
        break;
    case GETM:
        //Send Data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //Change to invalid
        state = MOESIF_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_snoop_O (Mreq *request)
{
        fprintf(stderr, "4\n");

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
        state = MOESIF_CACHE_I;
        break;
    case DATA: 
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }

}

inline void MOESIF_protocol::do_snoop_M (Mreq *request)
{
        fprintf(stderr, "5\n");

    switch (request->msg) {
    case GETS:
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //now owner of modified data
        state = MOESIF_CACHE_O;
        break;
    case GETM:
        //give data
        set_shared_line();
        send_DATA_on_bus(request->addr,request->src_mid);
        //now invalidated
        state = MOESIF_CACHE_I;
        break;
    case DATA:
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_IS (Mreq *request)
{
        fprintf(stderr, "6\n");

    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //send data
        send_DATA_to_proc(request->addr);
        //Determine state based on whether it came from a shared state
        if (get_shared_line()){
            state = MOESIF_CACHE_S;
        } else{
            state = MOESIF_CACHE_E;            
        }
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_IM (Mreq *request)
{
        fprintf(stderr, "7\n");

    switch (request->msg) {
    case GETS:
    case GETM:
        break;
    case DATA:
        //get data and set to modified
        send_DATA_to_proc(request->addr);
        //Change to modify state
        state = MOESIF_CACHE_M;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

inline void MOESIF_protocol::do_snoop_SM (Mreq *request)
{
        fprintf(stderr, "8\n");

    switch (request->msg) {
    case GETS:
        if(isForward==1 && !get_shared_line()){
            //give data
            send_DATA_on_bus(request->addr,request->src_mid);
        }

        //Tell it is shared currently
        set_shared_line();
    case GETM:

        if((isOwner==1||isForward==1) && !get_shared_line()){
            //give data
            set_shared_line();
            send_DATA_on_bus(request->addr,request->src_mid);
        }

        break;
    case DATA:
        //Reset flag
        isOwner = 0;
        isForward = 0;

        //Get data
        send_DATA_to_proc(request->addr);
        
        //Change to modify state
        state = MOESIF_CACHE_M;
        break;
    default:
        request->print_msg (my_table->moduleID, "ERROR");
        fatal_error ("Client: State shouldn't see this message\n");
    }
}

