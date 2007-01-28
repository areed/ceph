// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*- 
/*
 * Ceph - scalable distributed file system
 *
 * Copyright (C) 2004-2006 Sage Weil <sage@newdream.net>
 *
 * This is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1, as published by the Free Software 
 * Foundation.  See file COPYING.
 * 
 */

#include <sys/stat.h>
#include <iostream>
#include <string>
using namespace std;

#include "config.h"

#include "client/SyntheticClient.h"
#include "client/Client.h"
#include "client/fuse.h"

#include "msg/SimpleMessenger.h"

#include "common/Timer.h"
       
#include <envz.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv, char *envp[]) {

  //cerr << "cfuse starting " << myrank << "/" << world << endl;
  vector<char*> args;
  argv_to_vec(argc, argv, args);
  parse_config_options(args);
  parse_syn_options(args);   // for SyntheticClient

  // args for fuse
  vec_to_argv(args, argc, argv);

  // load monmap
  MonMap monmap;
  int r = monmap.read(".ceph_monmap");
  assert(r >= 0);

  // start up network
  rank.start_rank();

  // start client
  Client *client = new Client(rank.register_entity(MSG_ADDR_CLIENT_NEW), &monmap);
  client->init();
    
  // start syntheticclient
  SyntheticClient *syn = new SyntheticClient(client);

  // start up fuse
  // use my argc, argv (make sure you pass a mount point!)
  cout << "mounting" << endl;
  client->mount();
  
  cout << "starting syn client" << endl;
  syn->start_thread();

  // wait
  syn->join_thread();

  // unmount
  client->unmount();
  cout << "unmounted" << endl;
  client->shutdown();
  
  delete client;
  
  // wait for messenger to finish
  rank.wait();
  
  return 0;
}

