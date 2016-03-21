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

lpn_t* build_wb(void);
lpn_t* build_cpu1_read(void);
lpn_t* build_cpu1_write(void);
lpn_t* build_cpu0_read(void);
lpn_t* build_cpu0_write(void);


struct flow_instance_t
{
    lpn_t* flow_inst;
    config_t cfg;
    
    flow_instance_t() {
        flow_inst = nullptr;
        cfg = null_cfg;
    }
    
    flow_instance_t(const flow_instance_t& other) {
        flow_inst = other.flow_inst;
        cfg = other.cfg;
    }
    
    bool operator==(const flow_instance_t& other) {
        
        return (flow_inst->get_flow_name() == other.flow_inst->get_flow_name() &&cfg == other.cfg);
    }
    
    flow_instance_t& operator=(const flow_instance_t& other) {
        flow_inst = other.flow_inst;
        cfg = other.cfg;
        return *this;
    }
    
};
struct scenario_t{
    uint32_t read1;
    uint32_t read0;
    uint32_t write0;
    uint32_t write1;
    uint32_t wb;
    vector<flow_instance_t> active_t;
    
    scenario_t(){
        read0=0;
        read1=0;
        write0=0;
        write1=0;
        wb=0;
    }
    
};
//typedef vector<flow_instance_t> scenario_t;

void print_scenario(const scenario_t& sce)
{
    vector<flow_instance_t> scen=sce.active_t;
    for (uint32_t i = 0; i < scen.size(); i++) {
        const flow_instance_t& f = scen.at(i);
        
        uint32_t cfg = f.cfg;
        string cfg_str;
        bool cfg_convert_begin = true;
        for (uint32_t i = 0; i < 32; i++) {
            if ((cfg & 1) == 1 ) {
                if (cfg_convert_begin) {
                    cfg_str = to_string(i);
                    cfg_convert_begin = false;
                }
                else
                    cfg_str += " " + to_string(i);
            }
            cfg = cfg >> 1;
        }
        cfg_str = "<" + cfg_str + ">";
        
        cout << "(" << f.flow_inst->get_flow_name() << "  " << cfg_str << ")  ";
    }
    cout << endl;
}


void print_scenario(const vector<lpn_t*> flow_spec, const scenario_t& sce)
{
    vector<flow_instance_t> scen=sce.active_t;

    vector<uint32_t> flow_inst_cnt;
    flow_inst_cnt.push_back(sce.write0);
    flow_inst_cnt.push_back(sce.read0);
    flow_inst_cnt.push_back(sce.write1);
    flow_inst_cnt.push_back(sce.read1);
    flow_inst_cnt.push_back(sce.wb);
    
    
    for (uint32_t i = 0; i < scen.size(); i++) {
        const flow_instance_t& f = scen.at(i);
        uint32_t cfg = f.cfg;
        string cfg_str;
        bool cfg_convert_begin = true;
        for (uint32_t i = 0; i < 32; i++) {
            if ((cfg & 1) == 1 ) {
                if (cfg_convert_begin) {
                    cfg_str = to_string(i);
                    cfg_convert_begin = false;
                }
                else
                    cfg_str += " " + to_string(i);
            }
            cfg = cfg >> 1;
        }
        cfg_str = "<" + cfg_str + ">";
        uint32_t flow_index = f.flow_inst->get_index();
        ++flow_inst_cnt.at(flow_index);
        cout << "\t(" << f.flow_inst->get_flow_name() << " \t " << cfg_str<< ")\t";
        cout << endl;
    }
    
    cout << "***  # of flow instances:" << endl;
    for (uint32_t i = 0; i < flow_inst_cnt.size(); i++) {
        cout<<"before f"<<endl;
        lpn_t* flow = flow_spec.at(i);
        cout << "\t" << flow->get_flow_name() << ": \t" << flow_inst_cnt.at(flow->get_index()) << endl;
    }
    
    cout << endl;
}

std::hash<std::string> str_hash;

bool equalscen(const scenario_t &x, const scenario_t &y){
    if(x.read0!=y.read0||x.read1!=y.read1||x.write0!=y.write0||x.write1!=y.write1)
        return false;
    if(x.active_t.size()!=y.active_t.size())
        return false;
    for(uint32_t i=0;i<x.active_t.size();i++){
        if(x.active_t.at(i).flow_inst->get_flow_name()!=y.active_t.at(i).flow_inst->get_flow_name())
            return false;
        
        if(x.active_t.at(i).cfg!=y.active_t.at(i).cfg)
            return false;
        
        }
    
    
    return true;
}
vector<scenario_t> dscen(const vector<scenario_t> &vec){
//sort( vec.begin(), vec.end() );
   // vec.erase(vec.begi
   vector<scenario_t> rst;
    rst.push_back(vec.at(0));
    for(uint32_t i=1;i<vec.size();i++){
        bool flag=true;
        for(uint32_t j=0; j< rst.size(); j++){
            if(equalscen(vec.at(i),rst.at(j))==false);
            else{
                flag=false;
                break;}
        }
        if (flag==true)
        {rst.push_back(vec.at(i));
        }
    }
    return rst;
}

string cfg_str_c(const uint32_t& xcfg){
    uint32_t cfg=xcfg;
    string cfg_str;
    bool cfg_convert_begin = true;
    for (uint32_t i = 0; i < 32; i++) {
        if ((cfg & 1) == 1 ) {
            if (cfg_convert_begin) {
                cfg_str = to_string(i);
                cfg_convert_begin = false;
            }
            else
                cfg_str += " " + to_string(i);
        }
        cfg = cfg >> 1;
    }
    return cfg_str;
}

int main(int argc, char *argv[]) {
    struct rusage r_usage;
    
    // Build flow specification
    vector<lpn_t*> flow_spec;
    vector<uint32_t> noneed_monitors;
    
    lpn_t* cpu0_read = build_cpu0_read();
    lpn_t* cpu1_read=build_cpu1_read();
    lpn_t* cpu0_write=build_cpu0_write();
    lpn_t* cpu1_write=build_cpu1_write();
    lpn_t* write_back=build_wb();
    
    
    flow_spec.push_back(cpu0_read);
    cpu0_read->set_index(0);
    flow_spec.push_back(cpu1_read);//write1
    cpu1_read->set_index(1);
    flow_spec.push_back(cpu0_write);
    cpu0_write->set_index(2);
    flow_spec.push_back(cpu1_write);
    cpu1_write->set_index(3);
    flow_spec.push_back(write_back);
    write_back->set_index(4);
    
    
    vector<uint32_t> flow_inst_cnt;
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    flow_inst_cnt.push_back(0);
    
    ofstream errorfile;
    errorfile.open ("erromsg.txt",ios::trunc);
    
    vector<message_t> trace;
    
    uint32_t srcs[18]={ cpu0 , cpu1 , cache0 , cache1 , cache0 , cache1 , membus , membus , cache0 , cache1 , membus , membus , cache0 , cache1 , membus , mem , membus,mem };
    uint32_t dests[18]={ cache0 , cache1 , cpu0 , cpu1 , membus , membus , cache0 , cache1 , membus , membus , membus , membus , cache0 , cache1 , mem , membus , mem ,membus};
    // Open input trace file
    ifstream trace_file(argv[1]);
    if (trace_file.is_open()) {
        std::string line;
        message_t new_msg;
        int pl[17];
        while (getline(trace_file, line)){
            // From each line, get the message information to create a new msg.
            for (int mm=0; mm<17; mm++) {
                pl[mm]=0;
            }
            uint32_t state=0;
            for (uint32_t i = 0; i < line.size(); i++)
                if (line.at(i) == ',') {
                    pl[state] = i;
                    state++;
                }
            new_msg.addr = NDEF;
            
            uint32_t j =0;
            string tmp_str = line.substr(0, pl[0]);
            //cout<<"part, lala "<<tmp_str<<endl;
            if (tmp_str.at(0)=='1'){
                new_msg.src = cpu0;
                new_msg.dest = cache0;
                //new_msg.addr = NDEF;
                if (tmp_str.substr(1,2)=="10")
                    new_msg.cmd = rd;
                else
                    new_msg.cmd = wt;
                cout<<"cmd: "<<new_msg.cmd<<endl;
                trace.push_back(new_msg);
            }

            for (j=1; j<17; j++) {
                tmp_str = line.substr(pl[j-1]+1,52);
                //cout<<"inside : "<<j <<" "<<tmp_str<<endl;
                if (tmp_str.at(0)=='1'){
                    new_msg.src = srcs[j];
                    new_msg.dest=dests[j];
                    if (j==10 or j==11 or j==12 or j==13)
                        new_msg.cmd=snp;
                    else if (j==8 or j==9 or j==16 or j==17)
                        new_msg.cmd=wb;
                    else if(tmp_str.substr(1,2)=="10")
                        new_msg.cmd=rd;
                    else
                        new_msg.cmd=wt;
                    //cout<<"cmd: "<<new_msg.cmd<<endl;
                    trace.push_back(new_msg);
                }
            }
        }
        cout<<"finished"<<endl;
        trace_file.close();
    }
    else {
        cout << "Unable to open file" << endl;
        return 1;
    }
    
    cout << "Info: read " << trace.size() << " messages." << endl;
    
    vector<scenario_t> s_stack;
    stack<uint32_t> tri_stack;
    
    s_stack.push_back(scenario_t());
    tri_stack.push(0);
    
    vector<pair< vector<scenario_t>,uint32_t> >  bad_scenario_vec;
    
    // Matching message in the trace to scenairos.
    bool match = false;
    while (tri_stack.size() != 0) {
        //for(uint32_t niuniu=0;niuniu<100;niuniu++){
        match=false;
        uint32_t tri = tri_stack.top();
        tri_stack.pop();
        
        // If index tri reaches the end of trace, store the current scenario.
        if (tri == trace.size()) {
            //break if a scenario is found to match all messages.
            break;
        }
        if (tri%10==0){
            
            
            cout<<"************************"<<endl;
            cout<<"dscen called, orig size"<<s_stack.size()<<endl;
            s_stack=dscen(s_stack);
            cout<<"************************"<<endl;
            
            
            cout<<"NEW size"<<s_stack.size()<<endl;
        }
        vector<scenario_t> tmp_s_stack=s_stack;
        message_t msg(trace.at(tri));
        cout << tri<<"***  " << msg.toString() <<"  "<< s_stack.size() <<endl << endl;
        
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
            //cout << "Info: new instance (" << new_f.flow_inst->get_flow_name() << ", " << new_f.inst_id << ") is created" << endl << flush;
            //cout << "Info: " << msg.toString() << "\t\t (" << new_f.flow_inst->get_flow_name() << ", " << new_f.inst_id << ")." << endl << flush;
            
            //cout << "+++  new scenario (0) pushed to stack" << endl;
            //print_scenario(new_scenario);
            
            
            
        }
        
        // Match the next message from trace against the current scenario.
        for(uint32_t ct=0;ct<s_stack.size();ct++)
        {
            
            scenario_t scenario = s_stack.at(ct);
            // Match the enw_msg against the existing flow instances.
            for (uint32_t i = 0; i < scenario.active_t.size(); i++) {
                const flow_instance_t& f = scenario.active_t.at(i);
                config_t new_cfg = f.flow_inst->accept(msg, f.cfg);
                if (new_cfg != null_cfg) {
                    uint32_t flow_index = f.flow_inst->get_index();
                    scenario_t new_scenario = scenario;
                    
                    string cfg_str=cfg_str_c(new_cfg);
                    if(cfg_str=="16" ||cfg_str=="17" || cfg_str=="31")
                    {
                        if(flow_index==0)
                            new_scenario.read0++;
                        else if(flow_index==1)
                            new_scenario.read0++;
                        else if(flow_index==2)
                            new_scenario.write0++;
                        else if(flow_index==3)
                            new_scenario.write1++;
                        else if(flow_index==4)
                            new_scenario.wb++;
                        
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
                    //cout<<"create new scenario: "<<i<< " "<<cfg_str_c(flow_spec_flag.at(i))<<endl;
                    scenario_t new_scenario = scenario;
                    flow_instance_t new_f;
                    new_f.flow_inst = flow_spec.at(i);
                    ++flow_inst_cnt.at(i);
                    new_f.cfg = flow_spec_flag.at(i);
                    new_scenario.active_t.push_back(new_f);
                    new_s_stack.push_back(new_scenario);
                    tri_stack.push(tri+1);
                    match = true;
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
        cout << "======================================" << endl;
    }
    errorfile.close();
    if (s_stack.size() > 0) {
        cout << endl
        << "***  Success -  the scenario that matches all messages is" << endl;
        s_stack=dscen(s_stack);
        for(uint32_t ctt=0;ctt<s_stack.size();ctt++){
            scenario_t good_scen = s_stack.at(ctt);
            print_scenario(flow_spec, good_scen);
            cout << endl;
        }
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
    getrusage(RUSAGE_SELF,&r_usage);
    printf("************************Memory usage = %ld\n",r_usage.ru_maxrss);
    return 0;
    
}

//write_back flow
lpn_t* build_wb(void){
    lpn_t* lpn = new lpn_t;
    
    lpn->set_flow_name("****cpu0 writeback******");
    
    message_t msg1;
    msg1.pre_cfg = (1<<0);
    msg1.post_cfg = (1 << 1);
    msg1.src = cache0;
    msg1.dest = membus;
    msg1.cmd = wb;
    msg1.addr = NDEF;
    lpn->insert_msg(msg1);
    
    
    message_t msg2;
    msg2.pre_cfg = (1<<0);
    msg2.post_cfg = (1 << 1);
    msg2.src = cache1;
    msg2.dest = membus;
    msg2.cmd = wb;
    msg2.addr = NDEF;
    lpn->insert_msg(msg2);
    
    
    message_t msg15;
    msg15.pre_cfg = (1<<1);
    msg15.post_cfg = (1 << 2);
    msg15.src = membus;
    msg15.dest = mem;
    msg15.cmd = wb;
    msg15.addr = NDEF;
    lpn->insert_msg(msg15);
    
    message_t msg16;
    msg16.pre_cfg = (1<<2);
    msg16.post_cfg = (1 << 17);
    msg16.src = mem;
    msg16.dest = membus;
    msg16.cmd = wb;
    msg16.addr = NDEF;
    lpn->insert_msg(msg16);
    
    
    lpn->set_init_cfg(1<<0);
    
    return lpn;
}

lpn_t* build_cpu1_read(void){
    lpn_t* lpn = new lpn_t;
    
    lpn->set_flow_name("****cpu1 read*******");
    
    message_t msg1;
    msg1.pre_cfg = (1<<0);
    msg1.post_cfg = (1 << 1);
    msg1.src = cpu1;
    msg1.dest = cache1;
    msg1.cmd = rd;
    msg1.addr = NDEF;
    lpn->insert_msg(msg1);
    
    
    message_t msg15;
    msg15.pre_cfg = (1<<1);
    msg15.post_cfg = (1 << 17);
    msg15.src = cache1;
    msg15.dest = cpu1;
    msg15.cmd = rd;
    msg15.addr = NDEF;
    lpn->insert_msg(msg15);
    
    
    message_t msg22;
    msg22.pre_cfg = (1<<1);
    msg22.post_cfg = (1 << 2);
    msg22.src = cache1;
    msg22.dest = membus;
    msg22.cmd = rd;
    msg22.addr = NDEF;
    lpn->insert_msg(msg22);
    
    message_t msg23;
    msg23.pre_cfg = (1<<2);
    msg23.post_cfg = (1<<3);
    msg23.src = membus;
    msg23.dest = cache0;
    msg23.cmd = snp;
    msg23.addr = NDEF;
    lpn->insert_msg(msg23);
    
    
    message_t msg24;
    msg24.pre_cfg = (1<<3);
    msg24.post_cfg = (1<<4);
    msg24.src = cache0;
    msg24.dest = cpu0;
    msg24.cmd = snp;
    msg24.addr = NDEF;
    lpn->insert_msg(msg24);
    
    
    message_t msg29;
    msg29.pre_cfg = (1<<4);
    msg29.post_cfg = (1<<7);
    msg29.src = membus;
    msg29.dest = cache1;
    msg29.cmd = rd;
    msg29.addr = NDEF;
    lpn->insert_msg(msg29);
    
    message_t msg25;
    msg25.pre_cfg = (1<<4);
    msg25.post_cfg = (1<<5);
    msg25.src = membus;
    msg25.dest = mem;
    msg25.cmd = rd;
    msg25.addr = NDEF;
    lpn->insert_msg(msg25);
    
    message_t msg26;
    msg26.pre_cfg = (1<<5);
    msg26.post_cfg = (1<<6);
    msg26.src = mem;
    msg26.dest = membus;
    msg26.cmd = rd;
    msg26.addr = NDEF;
    lpn->insert_msg(msg26);
    
    message_t msg27;
    msg27.pre_cfg = (1<<6);
    msg27.post_cfg = (1 <<7);
    msg27.src = membus;
    msg27.dest = cache1;
    msg27.cmd = rd;
    msg27.addr = NDEF;
    lpn->insert_msg(msg27);
    
    
    message_t msg28;
    msg28.pre_cfg = (1<<7);
    msg28.post_cfg = (1<<17);
    msg28.src = cache1;
    msg28.dest = cpu1;
    msg28.cmd = rd;
    msg28.addr = NDEF;
    lpn->insert_msg(msg28);
    
    lpn->set_init_cfg(1<<0);
    
    return lpn;
}

lpn_t* build_cpu1_write(void){
    lpn_t* lpn = new lpn_t;
    
    lpn->set_flow_name("****cpu1_write*******");
    
    message_t msg1;
    msg1.pre_cfg = (1<<0);
    msg1.post_cfg = (1 << 1);
    msg1.src = cpu1;
    msg1.dest = cache1;
    msg1.cmd = wt;
    msg1.addr = NDEF;
    lpn->insert_msg(msg1);
    
    
    message_t msg15;
    msg15.pre_cfg = (1<<1);
    msg15.post_cfg = (1 << 17);
    msg15.src = cache1;
    msg15.dest = cpu1;
    msg15.cmd = wt;
    msg15.addr = NDEF;
    lpn->insert_msg(msg15);
    
    
    message_t msg22;
    msg22.pre_cfg = (1<<1);
    msg22.post_cfg = (1 << 2);
    msg22.src = cache1;
    msg22.dest = membus;
    msg22.cmd = wt;
    msg22.addr = NDEF;
    lpn->insert_msg(msg22);
    
    message_t msg23;
    msg23.pre_cfg = (1<<2);
    msg23.post_cfg = (1<<3);
    msg23.src = membus;
    msg23.dest = cache0;
    msg23.cmd = snp;
    msg23.addr = NDEF;
    lpn->insert_msg(msg23);
    
    
    message_t msg24;
    msg24.pre_cfg = (1<<3);
    msg24.post_cfg = (1<<4);
    msg24.src = cache0;
    msg24.dest = cpu0;
    msg24.cmd = snp;
    msg24.addr = NDEF;
    lpn->insert_msg(msg24);
    
    
    message_t msg29;
    msg29.pre_cfg = (1<<4);
    msg29.post_cfg = (1<<7);
    msg29.src = membus;
    msg29.dest = cache1;
    msg29.cmd = wt;
    msg29.addr = NDEF;
    lpn->insert_msg(msg29);
    
    message_t msg25;
    msg25.pre_cfg = (1<<4);
    msg25.post_cfg = (1<<5);
    msg25.src = membus;
    msg25.dest = mem;
    msg25.cmd = wt;
    msg25.addr = NDEF;
    lpn->insert_msg(msg25);
    
    message_t msg26;
    msg26.pre_cfg = (1<<5);
    msg26.post_cfg = (1<<6);
    msg26.src = mem;
    msg26.dest = membus;
    msg26.cmd = wt;
    msg26.addr = NDEF;
    lpn->insert_msg(msg26);
    
    message_t msg27;
    msg27.pre_cfg = (1<<6);
    msg27.post_cfg = (1 <<7);
    msg27.src = membus;
    msg27.dest = cache1;
    msg27.cmd = wt;
    msg27.addr = NDEF;
    lpn->insert_msg(msg27);
    
    
    message_t msg28;
    msg28.pre_cfg = (1<<7);
    msg28.post_cfg = (1<<17);
    msg28.src = cache1;
    msg28.dest = cpu1;
    msg28.cmd = wt;
    msg28.addr = NDEF;
    lpn->insert_msg(msg28);
    
    lpn->set_init_cfg(1<<0);
    
    return lpn;
}

lpn_t* build_cpu0_read(void){
    lpn_t* lpn = new lpn_t;
    
    lpn->set_flow_name("****cpu0 read*******");
    
    message_t msg1;
    msg1.pre_cfg = (1<<0);
    msg1.post_cfg = (1 << 1);
    msg1.src = cpu0;
    msg1.dest = cache0;
    msg1.cmd = rd;
    msg1.addr = NDEF;
    lpn->insert_msg(msg1);
    
    
    message_t msg15;
    msg15.pre_cfg = (1<<1);
    msg15.post_cfg = (1 << 17);
    msg15.src = cache0;
    msg15.dest = cpu0;
    msg15.cmd = rd;
    msg15.addr = NDEF;
    lpn->insert_msg(msg15);
    
    
    message_t msg22;
    msg22.pre_cfg = (1<<1);
    msg22.post_cfg = (1 << 2);
    msg22.src = cache0;
    msg22.dest = membus;
    msg22.cmd = rd;
    msg22.addr = NDEF;
    lpn->insert_msg(msg22);
    
    message_t msg23;
    msg23.pre_cfg = (1<<2);
    msg23.post_cfg = (1<<3);
    msg23.src = membus;
    msg23.dest = cache1;
    msg23.cmd = snp;
    msg23.addr = NDEF;
    lpn->insert_msg(msg23);
    
    
    message_t msg24;
    msg24.pre_cfg = (1<<3);
    msg24.post_cfg = (1<<4);
    msg24.src = cache1;
    msg24.dest = cpu1;
    msg24.cmd = snp;
    msg24.addr = NDEF;
    lpn->insert_msg(msg24);
    
    
    message_t msg29;
    msg29.pre_cfg = (1<<4);
    msg29.post_cfg = (1<<7);
    msg29.src = membus;
    msg29.dest = cache0;
    msg29.cmd = rd;
    msg29.addr = NDEF;
    lpn->insert_msg(msg29);
    
    message_t msg25;
    msg25.pre_cfg = (1<<4);
    msg25.post_cfg = (1<<5);
    msg25.src = membus;
    msg25.dest = mem;
    msg25.cmd = rd;
    msg25.addr = NDEF;
    lpn->insert_msg(msg25);
    
    message_t msg26;
    msg26.pre_cfg = (1<<5);
    msg26.post_cfg = (1<<6);
    msg26.src = mem;
    msg26.dest = membus;
    msg26.cmd = rd;
    msg26.addr = NDEF;
    lpn->insert_msg(msg26);
    
    message_t msg27;
    msg27.pre_cfg = (1<<6);
    msg27.post_cfg = (1 <<7);
    msg27.src = membus;
    msg27.dest = cache0;
    msg27.cmd = rd;
    msg27.addr = NDEF;
    lpn->insert_msg(msg27);
    
    
    message_t msg28;
    msg28.pre_cfg = (1<<7);
    msg28.post_cfg = (1<<17);
    msg28.src = cache0;
    msg28.dest = cpu0;
    msg28.cmd = rd;
    msg28.addr = NDEF;
    lpn->insert_msg(msg28);
    
    lpn->set_init_cfg(1<<0);
    
    return lpn;
}

lpn_t* build_cpu0_write(void){
    lpn_t* lpn = new lpn_t;
    
    lpn->set_flow_name("****cpu0_write*******");
    
    message_t msg1;
    msg1.pre_cfg = (1<<0);
    msg1.post_cfg = (1 << 1);
    msg1.src = cpu0;
    msg1.dest = cache0;
    msg1.cmd = wt;
    msg1.addr = NDEF;
    lpn->insert_msg(msg1);
    
    
    message_t msg15;
    msg15.pre_cfg = (1<<1);
    msg15.post_cfg = (1 << 17);
    msg15.src = cache0;
    msg15.dest = cpu0;
    msg15.cmd = wt;
    msg15.addr = NDEF;
    lpn->insert_msg(msg15);
    
    
    message_t msg22;
    msg22.pre_cfg = (1<<1);
    msg22.post_cfg = (1 << 2);
    msg22.src = cache0;
    msg22.dest = membus;
    msg22.cmd = wt;
    msg22.addr = NDEF;
    lpn->insert_msg(msg22);
    
    message_t msg23;
    msg23.pre_cfg = (1<<2);
    msg23.post_cfg = (1<<3);
    msg23.src = membus;
    msg23.dest = cache1;
    msg23.cmd = snp;
    msg23.addr = NDEF;
    lpn->insert_msg(msg23);
    
    
    message_t msg24;
    msg24.pre_cfg = (1<<3);
    msg24.post_cfg = (1<<4);
    msg24.src = cache1;
    msg24.dest = cpu1;
    msg24.cmd = snp;
    msg24.addr = NDEF;
    lpn->insert_msg(msg24);
    
    
    message_t msg29;
    msg29.pre_cfg = (1<<4);
    msg29.post_cfg = (1<<7);
    msg29.src = membus;
    msg29.dest = cache0;
    msg29.cmd = wt;
    msg29.addr = NDEF;
    lpn->insert_msg(msg29);
    
    message_t msg25;
    msg25.pre_cfg = (1<<4);
    msg25.post_cfg = (1<<5);
    msg25.src = membus;
    msg25.dest = mem;
    msg25.cmd = wt;
    msg25.addr = NDEF;
    lpn->insert_msg(msg25);
    
    message_t msg26;
    msg26.pre_cfg = (1<<5);
    msg26.post_cfg = (1<<6);
    msg26.src = mem;
    msg26.dest = membus;
    msg26.cmd = wt;
    msg26.addr = NDEF;
    lpn->insert_msg(msg26);
    
    message_t msg27;
    msg27.pre_cfg = (1<<6);
    msg27.post_cfg = (1 <<7);
    msg27.src = membus;
    msg27.dest = cache0;
    msg27.cmd = wt;
    msg27.addr = NDEF;
    lpn->insert_msg(msg27);
    
    
    message_t msg28;
    msg28.pre_cfg = (1<<7);
    msg28.post_cfg = (1<<17);
    msg28.src = cache0;
    msg28.dest = cpu0;
    msg28.cmd = wt;
    msg28.addr = NDEF;
    lpn->insert_msg(msg28);
    
    lpn->set_init_cfg(1<<0);
    
    return lpn;
}


