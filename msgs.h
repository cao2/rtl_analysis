//
//  msgs.cpp
//  
//
//  Created by Yuting Cao on 4/7/16.
//
//

#include "msg_def.h"
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <fstream>

class msgs{


private:
    vector<message_t> trace;
public:
    msgs() {}
    uint32_t num_signals;
    uint32_t val, cmd_s, cmd_e, addr_s, addr_e, data_s, data_e;
    vector<uint32_t> source;
    vector<uint32_t> destination;
    bool cfg(){
        bool cfg_flag=true;
        
        ifstream cfg_file("cfg.txt");
        if (cfg_file.is_open()){
            std::string ln;
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="signal"){
                    if (getline(cfg_file,ln)){
                        num_signals=stol(ln);
                        
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error singal number!!!"<<endl;
                    }
                    break;
                }
            }
         
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="src"){
                    if (getline(cfg_file,ln)){
                        string buf;
                        stringstream ss(ln);
                        
                        while (ss >> buf){
                            source.push_back(stol(buf));
                        }
                        
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error src source!!!"<<endl;
                    }
                    break;
                }
            }
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="dest"){
                    if (getline(cfg_file,ln)){
                        string buf;
                        stringstream ss(ln);
                        
                        while (ss >> buf){
                            destination.push_back(stol(buf));
                        }
                        
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error destination source!!!"<<endl;
                    }
                    break;
                }
            }
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="val"){
                    if (getline(cfg_file,ln)){
                        string buf;
                        stringstream ss(ln);
                        ss >> buf;
                        val=stol(buf);
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error cmd !!!"<<endl;
                    }
                    break;
                }
            }
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="cmd"){
                    if (getline(cfg_file,ln)){
                        string buf;
                        stringstream ss(ln);
                        ss >> buf;
                        cmd_s=stol(buf);
                        ss>>buf;
                        cmd_e=stol(buf);
                        
                        
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error cmd !!!"<<endl;
                    }
                    break;
                }
            }
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="addr"){
                    if (getline(cfg_file,ln)){
                        string buf;
                        stringstream ss(ln);
                        ss >> buf;
                        addr_s=stol(buf);
                        ss>>buf;
                        addr_e=stol(buf);
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error addr !!!"<<endl;
                    }
                    break;
                }
            }
            while (getline(cfg_file, ln)&&cfg_flag==true){
                if (ln=="data"){
                    if (getline(cfg_file,ln)){
                        string buf;
                        stringstream ss(ln);
                        ss >> buf;
                        data_s=stol(buf);
                        ss>>buf;
                        data_e=stol(buf);
                    }
                    else{
                        cfg_flag=false;
                        cout<<"error data !!!"<<endl;
                    }
                }
            }
            if (cfg_flag==true){
                cout<<"config: "<<endl;
                cout<<"number of signals: "<<num_signals<<endl;
                cout<<"source"<<endl;
                for (uint32_t j: source) {
                    cout<<" . "<<j;
                }
                cout<<endl;
                cout<<"destination"<<endl;
                for (uint32_t j: destination) {
                    cout<<" . "<<j;
                }
                cout<<endl;
                cout<<"val: "<<val<<endl;
                
                cout<<"cmd: "<<cmd_s<<" to "<<cmd_e<<endl;
                cout<<"addr: "<<addr_s<<" to "<<addr_e<<endl;
                cout<<"data: "<<data_s<<" to "<<data_e<<endl;
            }
            
        }
        
        return cfg_flag;
    }
    void parse(char *filename){
        uint32_t srcs[18]={ cpu0 ,      cpu1 ,      cache0 ,    cache1 ,    cache0 ,    cache1 ,    membus ,    membus ,    cache0 ,    cache1 , membus , membus , cache0 , cache1 , membus , mem , membus,mem };
        uint32_t dests[18]={ cache0 ,   cache1 ,    cpu0 ,      cpu1 ,      membus ,    membus ,    cache0 ,    cache1 ,    membus ,    membus , cache0 , cache1 , membus , membus , mem , membus , mem ,membus};
        // Open input trace file

        
        
        
        ifstream trace_file(filename);
        ofstream msg_file;
        msg_file.open ("msgs.txt",ios::trunc);
        
        
        if (cfg()&&trace_file.is_open()) {
            std::string line;
            message_t new_msg;
            int pl[num_signals-1];
            int linenm=0;
            int num =0;
            while (getline(trace_file, line)){
                
                // From each line, get the message information to create a new msg.
                linenm++;
                uint32_t state=0;
                
                for (uint32_t i = 0; i < line.size(); i++)
                    if (line.at(i) == ',') {
                        pl[state] = i;
                        state++;
                    }
                //cout<<endl<<line<<endl;
                uint32_t j =0;
                string tmp_str = line.substr(0, 52);
                if (tmp_str.at(val)=='1'){
                    new_msg.src = cpu0;
                    new_msg.dest = cache0;
                    //new_msg //
                    if (tmp_str.substr(1,2)=="10")
                        new_msg.cmd = rd;
                    else if (tmp_str.substr(1,2)=="11")
                        new_msg.cmd =pwr;
                    else
                        new_msg.cmd = wt;
                    new_msg.addr = stol(tmp_str.substr(3,6));
                    //new_msg.addr=0;
                    msg_file<<new_msg.toString()<<"\n";
                    trace.push_back(new_msg);
                    //cout<<new_msg.toString()<<endl;

                    num++;
                }
                
                for (j=1; j<17; j++) {
                    tmp_str = line.substr(pl[j-1]+2,52);
                    //cout<<"start:"<<j<<":"<<tmp_str<<endl;
                    if (j==14 or j==15)
                        tmp_str = tmp_str.substr(1,52);
                    if (tmp_str.at(val)=='1'){
                        //cout<<"inside : "<<tmp_str<<endl;
                        new_msg.src = srcs[j];
                        new_msg.dest= dests[j];
                        if (j==10 or j==11 or j==12 or j==13)
                            new_msg.cmd=snp;
                        else if (j==8 or j==9 or j==16 )
                            new_msg.cmd=wb;
                        else if(tmp_str.substr(1,2)=="10")
                            new_msg.cmd=rd;
                        else if(tmp_str.substr(1,2)=="11")
                            new_msg.cmd=pwr;
                        else
                            new_msg.cmd=wt;
                        //write to msg file
                        msg_file<<new_msg.toString()<<"\n";
                        new_msg.addr = stol(tmp_str.substr(3,6));
                        //new_msg.addr=0;
                        trace.push_back(new_msg);
                        //cout<<new_msg.toString()<<endl;
                        num++;
                    }
                }
                
                tmp_str = line.substr(pl[16]+2,1);
                if (tmp_str.at(0)=='1') {
                    new_msg.src = srcs[17];
                    new_msg.dest = dests[17];
                    new_msg.cmd = wb;
                    msg_file<<new_msg.toString()<<"\n";
                    new_msg.addr = stol(tmp_str.substr(3,6));
                    //new_msg.addr=0;
                    trace.push_back(new_msg);
                    //cout<<new_msg.toString()<<endl;

                    num++;

                }
            }
            cout<<"finished"<<endl;
            trace_file.close();
            msg_file.close();
            
        }
        else {
            cout << "Unable to open file" << endl;
        }
        cout << "Info: read " << trace.size() << " messages." << endl;
    }
    
    vector<message_t> getMsgs(){
        return trace;
    }
};
