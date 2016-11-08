#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <stack>
#include "lpn.h"
#include <set>
#include <algorithm>    // std::sort
#include <math.h>
#include <stdlib.h>
#include "msgs.h"
#include "vcd_msg.h"
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>/* needed to define getpid() */
#include <stdio.h>/* needed for printf() */
#include "fuc.h"

using namespace std;


int main(int argc, char *argv[]) {
    unsigned int pid = getpid();
    uint32_t max=0;
    struct rusage usage;
    struct timeval start, end;
    getrusage(RUSAGE_SELF, &usage);
    start = usage.ru_stime;
    // Build flow specification
    vector<lpn_t*> flow_spec;
    vector<uint32_t> noneed_monitors;
    
    lpn_t* cpu0_read = build_cpu0_read();
    lpn_t* cpu1_read=build_cpu1_read();
    lpn_t* cpu0_write=build_cpu0_write();
    lpn_t* cpu1_write=build_cpu1_write();
    lpn_t* write_back0=build_wb0();
    lpn_t* write_back1=build_wb1();
    
    
    flow_spec.push_back(cpu0_read);
    cpu0_read->set_index(0);
    flow_spec.push_back(cpu1_read);//write1
    cpu1_read->set_index(1);
    flow_spec.push_back(cpu0_write);
    cpu0_write->set_index(2);
    flow_spec.push_back(cpu1_write);
    cpu1_write->set_index(3);
    flow_spec.push_back(write_back0);
    write_back0->set_index(4);
    flow_spec.push_back(write_back1);
    write_back1->set_index(5);
    
    vector<uint32_t> flow_inst_cnt;
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    
    ofstream errorfile;
    errorfile.open ("erromsg.txt",ios::trunc);
    
    vector<message_t> trace;
    cout<<"lala"<<endl;
    string filename(argv[1]);
    
    if (filename.substr(filename.size()-3).compare("vcd")==0){
        vcd_msg* readmsg= new vcd_msg;
        readmsg->parse(argv[1]);
        trace = readmsg->getMsgs();
        
    }
    else{
        msgs* readmsg= new msgs;
        readmsg->parse(argv[1]);
        trace = readmsg->getMsgs();
    }
    
    
    
    cout<<"read "<<trace.size()<<endl;
    vector<scenario_t> s_stack;
    //hash_set<scenario_t> s_stack;
    stack<uint32_t> tri_stack;
    
    s_stack.push_back(scenario_t());
    tri_stack.push(0);
    
    
    vector<pair< vector<scenario_t>,uint32_t> >  bad_scenario_vec;
    
    // Matching message in the trace to scenairos.
    bool match = false;
    while (tri_stack.size() != 0) {
        match=false;
        uint32_t tri = tri_stack.top();
        tri_stack.pop();
        
        // If index tri reaches the end of trace, store the current scenario.
        if (tri == trace.size()) {
            //break if a scenario is found to match all messages.
            break;
        }
        
        //if((tri%5==0 &&s_stack.size()>10)|| s_stack.size()>tri*tri*2){
        if((tri%5==0 )|| s_stack.size()>tri*tri*2){
            cout<<"************************"<<endl;
            cout<<"dscen called, orig size "<<s_stack.size()<<endl;
            s_stack=dscen(s_stack);
            cout<<"************************"<<endl;
            cout<<"NEW size: "<<s_stack.size()<<endl;
            
        }
        if (tri%100==0){
            
            getMemUsage(pid, argv[2]);
        }
        vector<scenario_t> tmp_s_stack=s_stack;
        message_t msg(trace.at(tri));
        cout << tri<<"***  " << msg.toString() <<"  "<<s_stack.size() <<endl << endl;
        
        
        vector<scenario_t> new_s_stack;
        
        vector<config_t> flow_spec_flag;
        
        //find out if new msg can create a new flow_inst
        for (uint32_t i = 0; i < flow_spec.size(); i++) {
            lpn_t* f = flow_spec.at(i);
            config_t new_cfg = f->accept(msg);
            if (new_cfg != null_cfg)
                flow_spec_flag.push_back(new_cfg);
            else
                flow_spec_flag.push_back(99);
        }
        bool flag_inorder=false;
        vector<flow_instance_t> inst_set;
         // Match the next message from trace against the current scenario.
        for(uint32_t ct=0;ct<s_stack.size();ct++)
        {
            scenario_t scenario = s_stack.at(ct);
            // Match the enw_msg against the existing flow instances.
            for (uint32_t i = 0; i < scenario.active_t.size(); i++) {
                const flow_instance_t& f = scenario.active_t.at(i);
                bool inst_flg=false;
                for (uint32_t inst_ct=0; inst_ct<inst_set.size(); inst_ct++) {
                    flow_instance_t tmp_inst=inst_set.at(inst_ct);
                    if (tmp_inst.cfg==f.cfg && tmp_inst.flow_inst->get_index()==f.flow_inst->get_index()) {
                        inst_flg=true;
                    }
                }
                
                
                config_t new_cfg;
                if (inst_flg==false) {
                    inst_set.push_back(f);
                    new_cfg = f.flow_inst->accept(msg, f.cfg);
                }
                else
                    new_cfg = null_cfg;

                
                if ((new_cfg != null_cfg && f.addr == msg.addr)||(new_cfg != null_cfg && (f.flow_inst->get_index()==4 ||f.flow_inst->get_index()==5) )) {
                    flag_inorder=true;
                    uint32_t flow_index = f.flow_inst->get_index();
                    scenario_t new_scenario = scenario;
                    string cfg_str=cfg_str_c(new_cfg);
                    if(cfg_str=="17")
                    {
                        if(flow_index==0)
                            new_scenario.read0++;
                        else if(flow_index==1)
                            new_scenario.read1++;
                        else if(flow_index==2)
                            new_scenario.write0++;
                        else if(flow_index==3)
                            new_scenario.write1++;
                        else if(flow_index==4)
                            new_scenario.wb0++;
                        else if(flow_index==5)
                            new_scenario.wb1++;
                        new_scenario.order_finish.push_back(flow_index);
                        new_scenario.order_addr.push_back(f.addr);
                        new_scenario.active_t.erase(new_scenario.active_t.begin()+i);
                    }
                    else if(cfg_str =="16"){
                        if(flow_index==0)
                            new_scenario.read0_ch++;
                        else if(flow_index==1)
                            new_scenario.read1_ch++;
                        else if(flow_index==2)
                            new_scenario.write0_ch++;
                        else if(flow_index==3)
                            new_scenario.write1_ch++;
                        new_scenario.order_finish.push_back(flow_index+6);
                        new_scenario.order_addr.push_back(f.addr);
                        new_scenario.active_t.erase(new_scenario.active_t.begin()+i);
                    }
                    else{
                        new_scenario.active_t.at(i).cfg = new_cfg;
                    }
                    match = true;
                    new_s_stack.push_back(new_scenario);
                    tri_stack.push(tri+1);
                    //cout << "Info: " << msg.toString() << "\t\t (" << f.flow_inst->get_flow_name() << ", " << f.inst_id << ")." << endl << flush;
                    //cout << "+++  new scenario (1) pushed to stack" << endl;
                    //print_scenario(new_scenario);
                }
                
            }
            
            // Create a new flow instance to match msg.
            for(uint32_t i=0;i<flow_spec_flag.size();i++){
                if(flow_spec_flag.at(i)!=99){
                    scenario_t new_scenario = scenario;
                    flow_instance_t new_f;
                    new_f.flow_inst = flow_spec.at(i);
                    ++flow_inst_cnt.at(i);
                    new_f.cfg = flow_spec_flag.at(i);
                    new_f.addr = msg.addr;
                    new_scenario.active_t.push_back(new_f);
                    new_s_stack.push_back(new_scenario);
                    tri_stack.push(tri+1);
                    match = true;
                    //cout<<"new flow: "<< flow_spec.at(i)->get_flow_name() << "adds: "<< new_f.addr<<endl;
                }
            }
            
            
        }
        
        if (match == false) {
            tri_stack.push(tri+1);
            cout << "Info: " << trace.at(tri).toString() << " not matched, backtrack." << endl;
            pair< vector<scenario_t>,uint32_t> tmp_bad;
            tmp_bad.first=tmp_s_stack;
            tmp_bad.second=tri;
            bad_scenario_vec.push_back(tmp_bad);
            break;
            errorfile<<trace.at(tri).toString()<<"line #:"<<tri<<"\n";
            
        }
        else{
            s_stack=new_s_stack;
        }
        if (s_stack.size()>max)
        {
            max= s_stack.size();
        }
        cout << "======================================" << endl;
    }
    errorfile.close();
    bool succ = false;
    if (s_stack.size() > 0) {
        s_stack=dscen(s_stack);
        for(uint32_t ctt=0;ctt<s_stack.size();ctt++){
            scenario_t good_scen = s_stack.at(ctt);
            if(good_scen.active_t.size()==0){
                cout << endl
                << "***************"<<endl<<
                "  Success -  the scenario that matches all messages is" << endl;
                succ = true;
            }
            cout<<"possiblity #"<<ctt<<endl;
            print_scenario(flow_spec, good_scen);
            cout << endl;
            
            
        }
        
        if (succ == false)
            cout<<"non of the scenarios have finished all its flow"<<endl;
        /**
         for(uint32_t ctt=0;ctt<s_stack.size();ctt++){
         scenario_t good_scen = s_stack.at(ctt);
         cout<<"possiblity #"<<ctt<<endl;
         print_scenario(flow_spec, good_scen);
         cout << endl;
         }
         **/
    }
    
    else if (bad_scenario_vec.size()>0) {
        cout << endl
        << "***  Failed - generating the partial scenarios" << endl;
        pair<vector<scenario_t>,uint32_t> bad_scen= bad_scenario_vec.at(0);
        uint32_t msg_idx = bad_scen.second;
        message_t msg = trace.at(msg_idx);
        cout << "***  the following partial scenario failed to match message (" << msg_idx << ") " << msg.toString() << endl;
        
        for(uint32_t ctt=0;ctt<s_stack.size();ctt++){
            scenario_t tmp_print=bad_scen.first.at(ctt);
            print_scenario(flow_spec, tmp_print);
            cout << endl;}
    }
    getrusage(RUSAGE_SELF, &usage);
    end = usage.ru_stime;
    printf("************************Memory usage = %ld KB\n",
           usage.ru_maxrss);
    printf("************************Time usage: %ld.%ds sec\n", end.tv_sec-start.tv_sec, end.tv_usec-start.tv_usec);
    cout<<"Maximum number of flow instances: "<<max<<endl;
    getMemUsage(pid, argv[2]);
    max_mem(argv[2]);
    return 0;
    
}

