/*
Copyright (c) 2012-2014 The SSDB Authors. All rights reserved.
Use of this source code is governed by a BSD-style license that can be
found in the LICENSE file.
*/
#include "version.h"
#include "util/log.h"
#include "util/string_util.h"
#include "serv.h"
#include "net/proc.h"
#include "net/server.h"
#include "proc_sys.h"
#include "proc_kv.h"
#include "proc_hash.h"
#include "proc_zset.h"
#include "proc_queue.h"

#define REG_PROC(c, f)     net->proc_map.set_proc(#c, f, proc_##c)

void SSDBServer::reg_procs(NetworkServer *net){
	REG_PROC(get, "rt");
	REG_PROC(set, "wt");
	REG_PROC(del, "wt");
	REG_PROC(setx, "wt");
	REG_PROC(setnx, "wt");
	REG_PROC(getset, "wt");
	REG_PROC(getbit, "rt");
	REG_PROC(setbit, "wt");
	REG_PROC(countbit, "rt");
	REG_PROC(substr, "rt");
	REG_PROC(getrange, "rt");
	REG_PROC(strlen, "rt");
	REG_PROC(bitcount, "rt");
	REG_PROC(incr, "wt");
	REG_PROC(decr, "wt");
	REG_PROC(scan, "rt");
	REG_PROC(rscan, "rt");
	REG_PROC(keys, "rt");
	REG_PROC(rkeys, "rt");
	REG_PROC(exists, "rt");
	REG_PROC(multi_exists, "rt");
	REG_PROC(multi_get, "rt");
	REG_PROC(multi_set, "wt");
	REG_PROC(multi_del, "wt");
	REG_PROC(ttl, "rt");
	REG_PROC(expire, "wt");

	REG_PROC(hsize, "rt");
	REG_PROC(hget, "rt");
	REG_PROC(hset, "wt");
	REG_PROC(hdel, "wt");
	REG_PROC(hincr, "wt");
	REG_PROC(hdecr, "wt");
	REG_PROC(hclear, "wt");
	REG_PROC(hgetall, "rt");
	REG_PROC(hscan, "rt");
	REG_PROC(hrscan, "rt");
	REG_PROC(hkeys, "rt");
	REG_PROC(hvals, "rt");
	REG_PROC(hlist, "rt");
	REG_PROC(hrlist, "rt");
	REG_PROC(hexists, "rt");
	REG_PROC(multi_hexists, "rt");
	REG_PROC(multi_hsize, "rt");
	REG_PROC(multi_hget, "rt");
	REG_PROC(multi_hset, "wt");
	REG_PROC(multi_hdel, "wt");
	REG_PROC(hfix, "wt");

	// because zrank may be extremly slow, execute in a seperate thread
	REG_PROC(zrank, "rt");
	REG_PROC(zrrank, "rt");
	REG_PROC(zrange, "rt");
	REG_PROC(zrrange, "rt");
	REG_PROC(redis_zrange, "rt");
	REG_PROC(redis_zrrange, "rt");
	REG_PROC(zsize, "rt");
	REG_PROC(zget, "rt");
	REG_PROC(zset, "wt");
	REG_PROC(zdel, "wt");
	REG_PROC(zincr, "wt");
	REG_PROC(zdecr, "wt");
	REG_PROC(zclear, "wt");
	REG_PROC(zfix, "wt");
	REG_PROC(zscan, "rt");
	REG_PROC(zrscan, "rt");
	REG_PROC(zkeys, "rt");
	REG_PROC(zlist, "rt");
	REG_PROC(zrlist, "rt");
	REG_PROC(zcount, "rt");
	REG_PROC(zsum, "rt");
	REG_PROC(zavg, "rt");
	REG_PROC(zremrangebyrank, "wt");
	REG_PROC(zremrangebyscore, "wt");
	REG_PROC(zexists, "rt");
	REG_PROC(multi_zexists, "rt");
	REG_PROC(multi_zsize, "rt");
	REG_PROC(multi_zget, "rt");
	REG_PROC(multi_zset, "wt");
	REG_PROC(multi_zdel, "wt");
	REG_PROC(zpop_front, "wt");
	REG_PROC(zpop_back, "wt");

	REG_PROC(qsize, "rt");
	REG_PROC(qfront, "rt");
	REG_PROC(qback, "rt");
	REG_PROC(qpush, "wt");
	REG_PROC(qpush_front, "wt");
	REG_PROC(qpush_back, "wt");
	REG_PROC(qpop, "wt");
	REG_PROC(qpop_front, "wt");
	REG_PROC(qpop_back, "wt");
	REG_PROC(qtrim_front, "wt");
	REG_PROC(qtrim_back, "wt");
	REG_PROC(qfix, "wt");
	REG_PROC(qclear, "wt");
	REG_PROC(qlist, "rt");
	REG_PROC(qrlist, "rt");
	REG_PROC(qslice, "rt");
	REG_PROC(qrange, "rt");
	REG_PROC(qget, "rt");
	REG_PROC(qset, "wt");

	REG_PROC(clear_binlog, "wt");
	REG_PROC(flushdb, "wt");

	REG_PROC(dump, "b");
	REG_PROC(sync140, "b");
	REG_PROC(slaveof, "w");
	REG_PROC(delslave, "w");
	REG_PROC(info, "rt");
	REG_PROC(version, "r");
	REG_PROC(dbsize, "rt");
	REG_PROC(resetcopy, "w");
	REG_PROC(stopsync, "w");
	REG_PROC(startsync, "w");
	// doing compaction in a reader thread, because we have only one
	// writer thread(for performance reason); we don't want to block writes
	REG_PROC(compact, "rt");

	REG_PROC(ignore_key_range, "r");
	REG_PROC(get_key_range, "r");
	REG_PROC(get_kv_range, "r");
	REG_PROC(set_kv_range, "r");
}


SSDBServer::SSDBServer(SSDB *ssdb, SSDB *meta, const Config &conf, NetworkServer *net){
	this->ssdb = (SSDBImpl *)ssdb;
	this->meta = meta;

	net->data = this;
	this->reg_procs(net);

	this->cfg = &conf;
	this->sync_speed = conf.get_num("replication.sync_speed");

	backend_dump = new BackendDump(this->ssdb);
	backend_sync = new BackendSync(this->ssdb, this->sync_speed);
	expiration = new ExpirationHandler(this->ssdb);
	
	{ // slaves
		const Config *repl_conf = conf.get("replication");
		if(repl_conf != NULL){
			std::vector<Config *> children = repl_conf->children;
			for(std::vector<Config *>::iterator it = children.begin(); it != children.end(); it++){
				Config *c = *it;
				if(c->key != "slaveof"){
					continue;
				}
				std::string ip = c->get_str("ip");
				int port = c->get_num("port");
				if(ip == ""){
					ip = c->get_str("host");
				}
				if(ip == "" || port <= 0 || port > 65535){
					continue;
				}
				bool is_mirror = false;
				std::string type = c->get_str("type");
				if(type == "mirror"){
					is_mirror = true;
				}else{
					type = "sync";
					is_mirror = false;
				}
				
				std::string id = c->get_str("id");
				std::string auth = c->get_str("auth");
				int recv_timeout = c->get_num("recv_timeout");
				
				log_info("slaveof: %s:%d, type: %s", ip.c_str(), port, type.c_str());
				this->slaveof(id, ip, port, auth, 0, "", is_mirror, recv_timeout);
			}
		}
	}

	// load kv_range
	int ret = this->get_kv_range(&this->kv_range_s, &this->kv_range_e);
	if(ret == -1){
		log_fatal("load key_range failed!");
		exit(1);
	}
	log_info("key_range.kv: \"%s\", \"%s\"",
		str_escape(this->kv_range_s).c_str(),
		str_escape(this->kv_range_e).c_str()
		);
}

SSDBServer::~SSDBServer(){
	std::vector<Slave *>::iterator it;
	for(it = slaves.begin(); it != slaves.end(); it++){
		Slave *slave = *it;
		slave->stop();
		delete slave;
	}

	delete backend_dump;
	delete backend_sync;
	delete expiration;

	log_debug("SSDBServer finalized");
}

void SSDBServer::stopsync() {
	log_info("stopsync called");
	std::vector<Slave *>::iterator it;
	for(it = slaves.begin(); it != slaves.end(); it++){
		Slave *slave = *it;
		slave->last_seq = 0;
		slave->last_key = "";
		slave->save_status();
		slave->stop();
		delete slave;
	}
	slaves.clear();
	log_info("stopsync cleared slaves!");
	delete backend_sync;

	log_info("stopsync deleted backend_sync");
}

void SSDBServer::startsync() {
	backend_sync = new BackendSync(this->ssdb, this->sync_speed);
	log_info("startsync created new backend_sync");

	{ // slaves
		const Config *repl_conf = this->cfg->get("replication");
		if(repl_conf != NULL){
			std::vector<Config *> children = repl_conf->children;
			for(std::vector<Config *>::iterator it = children.begin(); it != children.end(); it++){
				Config *c = *it;
				if(c->key != "slaveof"){
					continue;
				}
				std::string ip = c->get_str("ip");
				int port = c->get_num("port");
				if(ip == ""){
					ip = c->get_str("host");
				}
				if(ip == "" || port <= 0 || port > 65535){
					continue;
				}
				bool is_mirror = false;
				std::string type = c->get_str("type");
				if(type == "mirror"){
					is_mirror = true;
				}else{
					type = "sync";
					is_mirror = false;
				}

				std::string id = c->get_str("id");
				int recv_timeout = c->get_num("recv_timeout");

				log_info("slaveof: %s:%d, type: %s", ip.c_str(), port, type.c_str());
				Slave *slave = new Slave(ssdb, meta, ip.c_str(), port, is_mirror);
				if(!id.empty()){
					slave->set_id(id);
				}
				if(recv_timeout > 0){
					slave->recv_timeout = recv_timeout;
				}
				slave->auth = c->get_str("auth");
				slave->start();
				slaves.push_back(slave);
			}
		}
	}

	log_info("startsync started new slaves");
}

void SSDBServer::resetcopy() {
	log_info("resetcopy called");
	std::vector<Slave *>::iterator it;
	for(it = slaves.begin(); it != slaves.end(); it++){
		Slave *slave = *it;
		slave->last_seq = 0;
		slave->last_key = "";
		slave->save_status();
		slave->stop();
		delete slave;
	}
	slaves.clear();
	log_info("resetcopy cleared slaves!2");
	delete backend_sync;

	log_info("resetcopy deleted backend_sync");
	backend_sync = new BackendSync(this->ssdb, this->sync_speed, false, true);
	log_info("resetcopy created new backend_sync");

	{ // slaves
		const Config *repl_conf = this->cfg->get("replication");
		if(repl_conf != NULL){
			std::vector<Config *> children = repl_conf->children;
			for(std::vector<Config *>::iterator it = children.begin(); it != children.end(); it++){
				Config *c = *it;
				if(c->key != "slaveof"){
					continue;
				}
				std::string ip = c->get_str("ip");
				int port = c->get_num("port");
				if(ip == ""){
					ip = c->get_str("host");
				}
				if(ip == "" || port <= 0 || port > 65535){
					continue;
				}
				bool is_mirror = false;
				std::string type = c->get_str("type");
				if(type == "mirror"){
					is_mirror = true;
				}else{
					type = "sync";
					is_mirror = false;
				}

				std::string id = c->get_str("id");
				int recv_timeout = c->get_num("recv_timeout");

				log_info("slaveof: %s:%d, type: %s", ip.c_str(), port, type.c_str());
				Slave *slave = new Slave(ssdb, meta, ip.c_str(), port, is_mirror);
				if(!id.empty()){
					slave->set_id(id);
				}
				if(recv_timeout > 0){
					slave->recv_timeout = recv_timeout;
				}
				slave->auth = c->get_str("auth");
				slave->start();
				slaves.push_back(slave);
			}
		}
	}

	log_info("resetcopy started new slaves");
}

int SSDBServer::slaveof(const std::string &id, const std::string &host, int port, const std::string &auth, uint64_t last_seq, const std::string &last_key, bool is_mirror, int recv_timeout){
	Slave *slave = new Slave(ssdb, meta, host.c_str(), port, is_mirror);
	if(!id.empty()){
		slave->set_id(id);
	}
	if(recv_timeout > 0){
		slave->recv_timeout = recv_timeout;
	}
	slave->last_seq = last_seq;
	slave->last_key = last_key;
	slave->auth = auth;
	slave->start();
	slaves.push_back(slave);
	return 0;
}

int SSDBServer::delslave(const std::string &id) {
	log_info("delslave called with id: %s", id.c_str());
	std::vector<Slave *>::iterator it;
	std::vector<Slave *>::iterator itselect;
	bool slavefound = false;

	for(it = slaves.begin(); it != slaves.end(); it++){
		Slave *slave = *it;

		log_info("iterating slave: %s", slave->id_.c_str());
		
		if (slave->id_ == id.c_str()) {
			slavefound = true;
			itselect = it;

			slave->last_seq = 0;
			slave->last_key = "";
			slave->save_status();
			slave->delslave();
			delete slave;
			log_info("deleted slave: %s", id.c_str());
		}
	}

	if ( slavefound ){
		log_info("Erased slave from vector: %s", id.c_str());
		slaves.erase(itselect);
	} 
	return 0;
}

int SSDBServer::set_kv_range(const std::string &start, const std::string &end){
	if(meta->hset("key_range", "kv_s", start) == -1){
		return -1;
	}
	if(meta->hset("key_range", "kv_e", end) == -1){
		return -1;
	}

	kv_range_s = start;
	kv_range_e = end;
	return 0;
}

int SSDBServer::get_kv_range(std::string *start, std::string *end){
	if(meta->hget("key_range", "kv_s", start) == -1){
		return -1;
	}
	if(meta->hget("key_range", "kv_e", end) == -1){
		return -1;
	}
	return 0;
}

bool SSDBServer::in_kv_range(const Bytes &key){
	if((this->kv_range_s.size() && this->kv_range_s >= key)
		|| (this->kv_range_e.size() && this->kv_range_e < key))
	{
		return false;
	}
	return true;
}

bool SSDBServer::in_kv_range(const std::string &key){
	if((this->kv_range_s.size() && this->kv_range_s >= key)
		|| (this->kv_range_e.size() && this->kv_range_e < key))
	{
		return false;
	}
	return true;
}


