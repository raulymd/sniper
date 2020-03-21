/*****************************************************************************
 *                                McPAT
 *                      SOFTWARE LICENSE AGREEMENT
 *            Copyright 2012 Hewlett-Packard Development Company, L.P.
 *                          All Rights Reserved
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.”
 *
 ***************************************************************************/

#include "io.h"
#include "parameter.h"
#include "array.h"
#include "const.h"
#include "logic.h"
#include "basic_circuit.h"
#include "arbiter.h"
#include <string.h>
#include <iostream>
#include <algorithm>
#include "XML_Parse.h"
#include <string.h>
#include <cmath>
#include <assert.h>
#include "sharedcache.h"

SharedCache::SharedCache(ParseXML* XML_interface, int ithCache_, InputParameter* interface_ip_, enum cache_level cacheL_)
:XML(XML_interface),
 ithCache(ithCache_),
 interface_ip(*interface_ip_),
 cacheL(cacheL_),
 dir_overhead(0)
{
  int idx;
  int tag, data;
  bool is_default, debug;
  enum Device_ty device_t;
  enum Core_type  core_t;
  double size, line, assoc, banks;
  if (cacheL==L2 && XML->sys.Private_L2)
  {
	  device_t=Core_device;
      core_t = (enum Core_type)XML->sys.core[ithCache].machine_type;
  }
  else
  {
	  device_t=LLC_device;
	  core_t = Inorder;
  }


  debug           = false;
  is_default=true;//indication for default setup
  if (XML->sys.Embedded)
  		{
  		interface_ip.wt                  =Global_30;
  		interface_ip.wire_is_mat_type = 0;
  		interface_ip.wire_os_mat_type = 1;
  		}
  	else
  		{
  		interface_ip.wt                  =Global;
  		interface_ip.wire_is_mat_type = 2;
  		interface_ip.wire_os_mat_type = 2;
  		}
  set_cache_param();

  //All lower level cache are physically indexed and tagged.
  size                             = cachep.capacity;
  line                             = cachep.blockW;
  assoc                            = cachep.assoc;
  banks                            = cachep.nbanks;
  if ((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory))
  {
	  assoc = 0;
	  tag   = XML->sys.physical_address_width + EXTRA_TAG_BITS;
	  interface_ip.num_search_ports    = 1;
  }
  else
  {
	  idx    					 	   = debug?9:int(ceil(log2(size/line/assoc)));
	  tag							   = debug?51:XML->sys.physical_address_width-idx-int(ceil(log2(line))) + EXTRA_TAG_BITS;
	  interface_ip.num_search_ports    = 0;
	  if (cachep.dir_ty==SBT)
	  {
		  dir_overhead = ceil(XML->sys.number_of_cores/8.0)*8/(cachep.blockW*8);
		  line = cachep.blockW*(1+ dir_overhead) ;
		  size = cachep.capacity*(1+ dir_overhead);

	  }
  }
//  if (XML->sys.first_level_dir==2)
//	  tag += int(XML->sys.domain_size + 5);
  interface_ip.specific_tag        = 1;
  interface_ip.tag_w               = tag;
  interface_ip.cache_sz            = (int)size;
  interface_ip.line_sz             = (int)line;
  interface_ip.assoc               = (int)assoc;
  interface_ip.nbanks              = (int)banks;
  interface_ip.out_w               = interface_ip.line_sz*8/2;
  interface_ip.access_mode         = 1;
  interface_ip.throughput          = cachep.throughput;
  interface_ip.latency             = cachep.latency;
  interface_ip.is_cache			 = true;
  interface_ip.pure_ram			 = false;
  interface_ip.pure_cam          = false;
  interface_ip.obj_func_dyn_energy = 0;
  interface_ip.obj_func_dyn_power  = 0;
  interface_ip.obj_func_leak_power = 0;
  interface_ip.obj_func_cycle_t    = 1;
  interface_ip.num_rw_ports        = 1;//lower level cache usually has one port.
  interface_ip.num_rd_ports        = 0;
  interface_ip.num_wr_ports        = 0;
  interface_ip.num_se_rd_ports     = 0;

//Raul
  bool keep_specific_hp_vdd 	= interface_ip.specific_hp_vdd;  
  bool keep_specific_lop_vdd 	= interface_ip.specific_lop_vdd;
  bool keep_specific_lstp_vdd 	= interface_ip.specific_lstp_vdd; 
  double keep_hp_Vdd 			= interface_ip.hp_Vdd;
  double keep_lop_Vdd  		= interface_ip.lop_Vdd;
  double keep_lstp_Vdd 		= interface_ip.lstp_Vdd;

  if(cacheL == L2 || cacheL == L3)
  {
 interface_ip.force_cache_config  =true;
 interface_ip.ndwl = 2;
 interface_ip.ndbl = 2;
 interface_ip.nspd = 0.5;
 interface_ip.ndcm =2 ;
 interface_ip.ndsam1 =1;
 interface_ip.ndsam2 =1;
	interface_ip.approx_mode = 1;
  }

  unicache.caches = new ArrayST(&interface_ip, cachep.name + "cache", device_t, true, core_t);
  unicache.area.set_area(unicache.area.get_area()+ unicache.caches->local_result.area);
  area.set_area(area.get_area()+ unicache.caches->local_result.area);
/*  
  cout << "-----EXACT results-----" << endl;
  cout << "readOp.dynamic\t\t" 			<< unicache.caches->local_result.power.readOp.dynamic 			<< endl;
  cout << "readOp.power_gated_leakage\t\t" 			<< unicache.caches->local_result.power.readOp.power_gated_leakage 			<< endl;
  cout << "readOp.gate_leakage\t" 		<< unicache.caches->local_result.power.readOp.gate_leakage 		<< endl;
  cout << "writeOp.dynamic\t\t" 		<< unicache.caches->local_result.power.writeOp.dynamic 			<< endl;
  cout << "writeOp.power_gated_leakage\t\t" 		<< unicache.caches->local_result.power.writeOp.power_gated_leakage 			<< endl;
  cout << "writeOp.gate_leakage\t" 		<< unicache.caches->local_result.power.writeOp.gate_leakage 		<< endl;
  */
  
 
  //approx_cache
  if(cacheL == L2 || cacheL == L3)	
  {
	  
	interface_ip.specific_hp_vdd = true;
	interface_ip.specific_lop_vdd = true;
	interface_ip.specific_lstp_vdd = true;
	interface_ip.hp_Vdd   = cacheL == L2 ? XML->sys.L2[ithCache].approxVdd:XML->sys.L3[ithCache].approxVdd;		//extracted by MemHierModel
	interface_ip.lop_Vdd  = cacheL == L2 ? XML->sys.L2[ithCache].approxVdd:XML->sys.L3[ithCache].approxVdd;		//extracted by MemHierModel
	interface_ip.lstp_Vdd = cacheL == L2 ? XML->sys.L2[ithCache].approxVdd:XML->sys.L3[ithCache].approxVdd;		//extracted by MemHierModel    
	//std::cout << "Cache lvl: " <<  "L2" << " approxVdd: " << interface_ip.hp_Vdd  << std::endl;
  } 
  unicache.apcaches = new ArrayST(&interface_ip, cachep.name + "apcache", device_t, true, core_t);
  /*
  cout << "-----APPROX results-----" << endl;
  cout << "readOp.dynamic\t\t" 			<< unicache.apcaches->local_result.power.readOp.dynamic 			<< endl;
  cout << "readOp.gated_leakage\t" 		<< unicache.apcaches->local_result.power.readOp.power_gated_leakage	<< endl;
  cout << "readOp.gate_leakage\t" 		<< unicache.apcaches->local_result.power.readOp.gate_leakage 		<< endl;
  cout << "readOp.long_ch_leakage\t" 	<< unicache.apcaches->local_result.power.readOp.longer_channel_leakage		<< endl;
  cout << "writeOp.dynamic\t\t" 		<< unicache.apcaches->local_result.power.writeOp.dynamic 			<< endl;
  cout << "writeOp.gated_leakage\t" 	<< unicache.apcaches->local_result.power.writeOp.power_gated_leakage<< endl;
  cout << "writeOp.gate_leakage\t" 		<< unicache.apcaches->local_result.power.writeOp.gate_leakage 		<< endl;
  cout << "writeOp.long_ch_leakage\t" 	<< unicache.apcaches->local_result.power.writeOp.longer_channel_leakage		<< endl;  
*/
  //exiting
  interface_ip.force_cache_config  =false; 
  interface_ip.approx_mode = 0;
  interface_ip.specific_hp_vdd = keep_specific_hp_vdd;
  interface_ip.specific_lop_vdd = keep_specific_lop_vdd;
  interface_ip.specific_lstp_vdd = keep_specific_lstp_vdd;
  interface_ip.hp_Vdd   = keep_hp_Vdd;		//extracted by MemHierModel
  interface_ip.lop_Vdd  = keep_lop_Vdd;		//extracted by MemHierModel
  interface_ip.lstp_Vdd = keep_lstp_Vdd;		//extracted by MemHierModel    
  

  if (!((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory)))
  {
	  tag							   = XML->sys.physical_address_width + EXTRA_TAG_BITS;
	  data							   = (XML->sys.physical_address_width) + int(ceil(log2(size/line))) + unicache.caches->l_ip.line_sz;
	  interface_ip.specific_tag        = 1;
	  interface_ip.tag_w               = tag;
	  interface_ip.line_sz             = int(ceil(data/8.0));//int(ceil(pow(2.0,ceil(log2(data)))/8.0));
	  interface_ip.cache_sz            = cachep.missb_size*interface_ip.line_sz;
	  interface_ip.assoc               = 0;
	  interface_ip.is_cache			   = true;
	  interface_ip.pure_ram			   = false;
	  interface_ip.pure_cam            = false;
	  interface_ip.nbanks              = 1;
	  interface_ip.out_w               = interface_ip.line_sz*8/2;
	  interface_ip.access_mode         = 0;
	  interface_ip.throughput          = cachep.throughput;//means cycle time
	  interface_ip.latency             = cachep.latency;//means access time
	  interface_ip.obj_func_dyn_energy = 0;
	  interface_ip.obj_func_dyn_power  = 0;
	  interface_ip.obj_func_leak_power = 0;
	  interface_ip.obj_func_cycle_t    = 1;
	  interface_ip.num_rw_ports    = 1;
	  interface_ip.num_rd_ports    = 0;
	  interface_ip.num_wr_ports    = 0;
	  interface_ip.num_se_rd_ports = 0;
	  interface_ip.num_search_ports    = 1;
	  unicache.missb = new ArrayST(&interface_ip, cachep.name + "MissB", device_t, true, core_t);
	  unicache.area.set_area(unicache.area.get_area()+ unicache.missb->local_result.area);
	  area.set_area(area.get_area()+ unicache.missb->local_result.area);
	  //fill buffer
	  tag							   = XML->sys.physical_address_width + EXTRA_TAG_BITS;
	  data							   = unicache.caches->l_ip.line_sz;
	  interface_ip.specific_tag        = 1;
	  interface_ip.tag_w               = tag;
	  interface_ip.line_sz             = data;//int(pow(2.0,ceil(log2(data))));
	  interface_ip.cache_sz            = data*cachep.fu_size ;
	  interface_ip.assoc               = 0;
	  interface_ip.nbanks              = 1;
	  interface_ip.out_w               = interface_ip.line_sz*8/2;
	  interface_ip.access_mode         = 0;
	  interface_ip.throughput          =  cachep.throughput;
	  interface_ip.latency             =  cachep.latency;
	  interface_ip.obj_func_dyn_energy = 0;
	  interface_ip.obj_func_dyn_power  = 0;
	  interface_ip.obj_func_leak_power = 0;
	  interface_ip.obj_func_cycle_t    = 1;
	  interface_ip.num_rw_ports    = 1;
	  interface_ip.num_rd_ports    = 0;
	  interface_ip.num_wr_ports    = 0;
	  interface_ip.num_se_rd_ports = 0;
	  unicache.ifb = new ArrayST(&interface_ip, cachep.name + "FillB", device_t, true, core_t);
	  unicache.area.set_area(unicache.area.get_area()+ unicache.ifb->local_result.area);
	  area.set_area(area.get_area()+ unicache.ifb->local_result.area);
	  //prefetch buffer
	  tag							   = XML->sys.physical_address_width + EXTRA_TAG_BITS;//check with previous entries to decide wthether to merge.
	  data							   = unicache.caches->l_ip.line_sz;//separate queue to prevent from cache polution.
	  interface_ip.specific_tag        = 1;
	  interface_ip.tag_w               = tag;
	  interface_ip.line_sz             = data;//int(pow(2.0,ceil(log2(data))));
	  interface_ip.cache_sz            = cachep.prefetchb_size*interface_ip.line_sz;
	  interface_ip.assoc               = 0;
	  interface_ip.nbanks              = 1;
	  interface_ip.out_w               = interface_ip.line_sz*8/2;
	  interface_ip.access_mode         = 0;
	  interface_ip.throughput          = cachep.throughput;
	  interface_ip.latency             = cachep.latency;
	  interface_ip.obj_func_dyn_energy = 0;
	  interface_ip.obj_func_dyn_power  = 0;
	  interface_ip.obj_func_leak_power = 0;
	  interface_ip.obj_func_cycle_t    = 1;
	  interface_ip.num_rw_ports    = 1;
	  interface_ip.num_rd_ports    = 0;
	  interface_ip.num_wr_ports    = 0;
	  interface_ip.num_se_rd_ports = 0;
	  unicache.prefetchb = new ArrayST(&interface_ip, cachep.name + "PrefetchB", device_t, true, core_t);
	  unicache.area.set_area(unicache.area.get_area()+ unicache.prefetchb->local_result.area);
	  area.set_area(area.get_area()+ unicache.prefetchb->local_result.area);
	  //WBB
	  tag							   = XML->sys.physical_address_width + EXTRA_TAG_BITS;
	  data							   = unicache.caches->l_ip.line_sz;
	  interface_ip.specific_tag        = 1;
	  interface_ip.tag_w               = tag;
	  interface_ip.line_sz             = data;
	  interface_ip.cache_sz            = cachep.wbb_size*interface_ip.line_sz;
	  interface_ip.assoc               = 0;
	  interface_ip.nbanks              = 1;
	  interface_ip.out_w               = interface_ip.line_sz*8/2;
	  interface_ip.access_mode         = 0;
	  interface_ip.throughput          = cachep.throughput;
	  interface_ip.latency             = cachep.latency;
	  interface_ip.obj_func_dyn_energy = 0;
	  interface_ip.obj_func_dyn_power  = 0;
	  interface_ip.obj_func_leak_power = 0;
	  interface_ip.obj_func_cycle_t    = 1;
	  interface_ip.num_rw_ports    = 1;
	  interface_ip.num_rd_ports    = 0;
	  interface_ip.num_wr_ports    = 0;
	  interface_ip.num_se_rd_ports = 0;
	  unicache.wbb = new ArrayST(&interface_ip, cachep.name + "WBB", device_t, true, core_t);
	  unicache.area.set_area(unicache.area.get_area()+ unicache.wbb->local_result.area);
	  area.set_area(area.get_area()+ unicache.wbb->local_result.area);
  }
  //Raul -> deleted compents


}


void SharedCache::computeEnergy(bool is_tdp)
{
	double homenode_data_access = (cachep.dir_ty==SBT)? 0.9:1.0;
	double approx_percent = 0;
	if (is_tdp)
	{
		//Raul
		if(cacheL == L2)
			approx_percent = XML->sys.L2[ithCache].approx_ratio;
		else if(cacheL == L3)
			approx_percent = XML->sys.L3[ithCache].approx_ratio;
		
		if (!((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory)))
		{
			//init stats for Peak
			unicache.caches->stats_t.readAc.access  = .67*unicache.caches->l_ip.num_rw_ports*cachep.duty_cycle*homenode_data_access;
			unicache.caches->stats_t.readAc.miss    = 0;
			unicache.caches->stats_t.readAc.hit     = unicache.caches->stats_t.readAc.access - unicache.caches->stats_t.readAc.miss;
			unicache.caches->stats_t.writeAc.access = .33*unicache.caches->l_ip.num_rw_ports*cachep.duty_cycle*homenode_data_access;
			unicache.caches->stats_t.writeAc.miss   = 0;
			unicache.caches->stats_t.writeAc.hit    = unicache.caches->stats_t.writeAc.access -	unicache.caches->stats_t.writeAc.miss;
			unicache.caches->tdp_stats = unicache.caches->stats_t;

			if (cachep.dir_ty==SBT)
			{
				homenode_stats_t.readAc.access  = .67*unicache.caches->l_ip.num_rw_ports*cachep.dir_duty_cycle*(1-homenode_data_access);
				homenode_stats_t.readAc.miss    = 0;
				homenode_stats_t.readAc.hit     = homenode_stats_t.readAc.access - homenode_stats_t.readAc.miss;
				homenode_stats_t.writeAc.access  = .67*unicache.caches->l_ip.num_rw_ports*cachep.dir_duty_cycle*(1-homenode_data_access);
				homenode_stats_t.writeAc.miss   = 0;
				homenode_stats_t.writeAc.hit    = homenode_stats_t.writeAc.access -	homenode_stats_t.writeAc.miss;
				homenode_tdp_stats = homenode_stats_t;
			}

			unicache.missb->stats_t.readAc.access  = unicache.missb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.missb->stats_t.writeAc.access = unicache.missb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.missb->tdp_stats = unicache.missb->stats_t;

			unicache.ifb->stats_t.readAc.access  = unicache.ifb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.ifb->stats_t.writeAc.access = unicache.ifb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.ifb->tdp_stats = unicache.ifb->stats_t;

			unicache.prefetchb->stats_t.readAc.access  = unicache.prefetchb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.prefetchb->stats_t.writeAc.access = unicache.ifb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.prefetchb->tdp_stats = unicache.prefetchb->stats_t;

			unicache.wbb->stats_t.readAc.access  = unicache.wbb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.wbb->stats_t.writeAc.access = unicache.wbb->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.wbb->tdp_stats = unicache.wbb->stats_t;
		}
		else
		{
			unicache.caches->stats_t.readAc.access  = unicache.caches->l_ip.num_search_ports*cachep.duty_cycle;
			unicache.caches->stats_t.readAc.miss    = 0;
			unicache.caches->stats_t.readAc.hit     = unicache.caches->stats_t.readAc.access - unicache.caches->stats_t.readAc.miss;
			unicache.caches->stats_t.writeAc.access = 0;
			unicache.caches->stats_t.writeAc.miss   = 0;
			unicache.caches->stats_t.writeAc.hit    = unicache.caches->stats_t.writeAc.access -	unicache.caches->stats_t.writeAc.miss;
			unicache.caches->tdp_stats = unicache.caches->stats_t;

		}

	}
	else
	{
		//init stats for runtime power (RTP)
		if (cacheL==L2)
		{
			unicache.caches->stats_t.readAc.access  = XML->sys.L2[ithCache].read_accesses;
			unicache.caches->stats_t.readAc.miss    = XML->sys.L2[ithCache].read_misses;
			unicache.caches->stats_t.readAc.hit     = unicache.caches->stats_t.readAc.access - unicache.caches->stats_t.readAc.miss;
			unicache.caches->stats_t.writeAc.access = XML->sys.L2[ithCache].write_accesses;
			unicache.caches->stats_t.writeAc.miss   = XML->sys.L2[ithCache].write_misses;
			unicache.caches->stats_t.writeAc.hit    = unicache.caches->stats_t.writeAc.access -	unicache.caches->stats_t.writeAc.miss;
			unicache.caches->rtp_stats = unicache.caches->stats_t;
			//Raul
			unicache.apcaches->stats_t.readAc.access  = XML->sys.L2[ithCache].approx_read_accesses;
			unicache.apcaches->stats_t.readAc.miss    = XML->sys.L2[ithCache].approx_read_misses;
			unicache.apcaches->stats_t.readAc.hit     = unicache.apcaches->stats_t.readAc.access - unicache.apcaches->stats_t.readAc.miss;
			unicache.apcaches->stats_t.writeAc.access = XML->sys.L2[ithCache].approx_write_accesses;
			unicache.apcaches->stats_t.writeAc.miss   = XML->sys.L2[ithCache].approx_write_misses;
			unicache.apcaches->stats_t.writeAc.hit    = unicache.apcaches->stats_t.writeAc.access -	unicache.apcaches->stats_t.writeAc.miss;
			approx_percent = XML->sys.L2[ithCache].approx_ratio;
			unicache.apcaches->rtp_stats = unicache.apcaches->stats_t;
			
/*			//Raul
      cout << "XML->sys.L2[ithCache].approx_read_accesses: " << XML->sys.L2[ithCache].approx_read_accesses << endl;
      cout << "XML->sys.L2[ithCache].approx_write_accesses: " << XML->sys.L2[ithCache].approx_write_accesses << endl;
       cout << "XML->sys.L2[ithCache].approx_read_misses: " << XML->sys.L2[ithCache].approx_read_misses << endl;
       cout << "XML->sys.L2[ithCache].approx_write_misses: " << XML->sys.L2[ithCache].approx_write_misses << endl;
       cout << "XML->sys.L2[ithCache].approx_ratio: " << XML->sys.L2[ithCache].approx_ratio << endl;
       cout << "approx_percent: " << approx_percent << endl;
*/
			if (cachep.dir_ty==SBT)
			{
				homenode_rtp_stats.readAc.access  = XML->sys.L2[ithCache].homenode_read_accesses;
				homenode_rtp_stats.readAc.miss    = XML->sys.L2[ithCache].homenode_read_misses;
				homenode_rtp_stats.readAc.hit     = homenode_rtp_stats.readAc.access - homenode_rtp_stats.readAc.miss;
				homenode_rtp_stats.writeAc.access = XML->sys.L2[ithCache].homenode_write_accesses;
				homenode_rtp_stats.writeAc.miss   = XML->sys.L2[ithCache].homenode_write_misses;
				homenode_rtp_stats.writeAc.hit    = homenode_rtp_stats.writeAc.access -	homenode_rtp_stats.writeAc.miss;
			}
		}
		else if (cacheL==L3)
		{
			unicache.caches->stats_t.readAc.access  = XML->sys.L3[ithCache].read_accesses;
			unicache.caches->stats_t.readAc.miss    = XML->sys.L3[ithCache].read_misses;
			unicache.caches->stats_t.readAc.hit     = unicache.caches->stats_t.readAc.access - unicache.caches->stats_t.readAc.miss;
			unicache.caches->stats_t.writeAc.access = XML->sys.L3[ithCache].write_accesses;
			unicache.caches->stats_t.writeAc.miss   = XML->sys.L3[ithCache].write_misses;
			unicache.caches->stats_t.writeAc.hit    = unicache.caches->stats_t.writeAc.access -	unicache.caches->stats_t.writeAc.miss;
			unicache.caches->rtp_stats = unicache.caches->stats_t;
			//Raul
			unicache.apcaches->stats_t.readAc.access  = XML->sys.L3[ithCache].approx_read_accesses;
			unicache.apcaches->stats_t.readAc.miss    = XML->sys.L3[ithCache].approx_read_misses;
			unicache.apcaches->stats_t.readAc.hit     = unicache.apcaches->stats_t.readAc.access - unicache.apcaches->stats_t.readAc.miss;
			unicache.apcaches->stats_t.writeAc.access = XML->sys.L3[ithCache].approx_write_accesses;
			unicache.apcaches->stats_t.writeAc.miss   = XML->sys.L3[ithCache].approx_write_misses;
			unicache.apcaches->stats_t.writeAc.hit    = unicache.apcaches->stats_t.writeAc.access -	unicache.apcaches->stats_t.writeAc.miss;
			approx_percent = XML->sys.L3[ithCache].approx_ratio;
			unicache.apcaches->rtp_stats = unicache.apcaches->stats_t;			

      //Raul
      // cout << "XML->sys.L2[ithCache].approx_read_accesses: " << XML->sys.L2[ithCache].approx_read_accesses << endl;
      // cout << "XML->sys.L2[ithCache].approx_write_accesses: " << XML->sys.L2[ithCache].approx_write_accesses << endl;
      // cout << "XML->sys.L2[ithCache].approx_read_misses: " << XML->sys.L2[ithCache].approx_read_misses << endl;
      // cout << "XML->sys.L2[ithCache].approx_write_misses: " << XML->sys.L2[ithCache].approx_write_misses << endl;
      // cout << "XML->sys.L2[ithCache].approx_ratio: " << XML->sys.L2[ithCache].approx_ratio << endl;
      

			if (cachep.dir_ty==SBT)
			{
				homenode_rtp_stats.readAc.access  = XML->sys.L3[ithCache].homenode_read_accesses;
				homenode_rtp_stats.readAc.miss    = XML->sys.L3[ithCache].homenode_read_misses;
				homenode_rtp_stats.readAc.hit     = homenode_rtp_stats.readAc.access - homenode_rtp_stats.readAc.miss;
				homenode_rtp_stats.writeAc.access = XML->sys.L3[ithCache].homenode_write_accesses;
				homenode_rtp_stats.writeAc.miss   = XML->sys.L3[ithCache].homenode_write_misses;
				homenode_rtp_stats.writeAc.hit    = homenode_rtp_stats.writeAc.access -	homenode_rtp_stats.writeAc.miss;
			}
		}
		else if (cacheL==L1Directory)
		{
			unicache.caches->stats_t.readAc.access  = XML->sys.L1Directory[ithCache].read_accesses;
			unicache.caches->stats_t.readAc.miss    = XML->sys.L1Directory[ithCache].read_misses;
			unicache.caches->stats_t.readAc.hit     = unicache.caches->stats_t.readAc.access - unicache.caches->stats_t.readAc.miss;
			unicache.caches->stats_t.writeAc.access = XML->sys.L1Directory[ithCache].write_accesses;
			unicache.caches->stats_t.writeAc.miss   = XML->sys.L1Directory[ithCache].write_misses;
			unicache.caches->stats_t.writeAc.hit    = unicache.caches->stats_t.writeAc.access -	unicache.caches->stats_t.writeAc.miss;
			unicache.caches->rtp_stats = unicache.caches->stats_t;
		}
		else if (cacheL==L2Directory)
		{
			unicache.caches->stats_t.readAc.access  = XML->sys.L2Directory[ithCache].read_accesses;
			unicache.caches->stats_t.readAc.miss    = XML->sys.L2Directory[ithCache].read_misses;
			unicache.caches->stats_t.readAc.hit     = unicache.caches->stats_t.readAc.access - unicache.caches->stats_t.readAc.miss;
			unicache.caches->stats_t.writeAc.access = XML->sys.L2Directory[ithCache].write_accesses;
			unicache.caches->stats_t.writeAc.miss   = XML->sys.L2Directory[ithCache].write_misses;
			unicache.caches->stats_t.writeAc.hit    = unicache.caches->stats_t.writeAc.access -	unicache.caches->stats_t.writeAc.miss;
			unicache.caches->rtp_stats = unicache.caches->stats_t;
		}
		if (!((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory)))
		{   //Assuming write back and write-allocate cache

			unicache.missb->stats_t.readAc.access  = unicache.caches->stats_t.writeAc.miss ;
			unicache.missb->stats_t.writeAc.access = unicache.caches->stats_t.writeAc.miss;
			unicache.missb->rtp_stats = unicache.missb->stats_t;

			unicache.ifb->stats_t.readAc.access  = unicache.caches->stats_t.writeAc.miss;
			unicache.ifb->stats_t.writeAc.access = unicache.caches->stats_t.writeAc.miss;
			unicache.ifb->rtp_stats = unicache.ifb->stats_t;

			unicache.prefetchb->stats_t.readAc.access  = unicache.caches->stats_t.writeAc.miss;
			unicache.prefetchb->stats_t.writeAc.access = unicache.caches->stats_t.writeAc.miss;
			unicache.prefetchb->rtp_stats = unicache.prefetchb->stats_t;

			unicache.wbb->stats_t.readAc.access  = unicache.caches->stats_t.writeAc.miss;
			unicache.wbb->stats_t.writeAc.access = unicache.caches->stats_t.writeAc.miss;
			if (cachep.dir_ty==SBT)
			{
				unicache.missb->stats_t.readAc.access  += homenode_rtp_stats.writeAc.miss;
				unicache.missb->stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
				unicache.missb->rtp_stats = unicache.missb->stats_t;

				unicache.missb->stats_t.readAc.access  += homenode_rtp_stats.writeAc.miss;
				unicache.missb->stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
				unicache.missb->rtp_stats = unicache.missb->stats_t;

				unicache.ifb->stats_t.readAc.access  += homenode_rtp_stats.writeAc.miss;
				unicache.ifb->stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
				unicache.ifb->rtp_stats = unicache.ifb->stats_t;

				unicache.prefetchb->stats_t.readAc.access  += homenode_rtp_stats.writeAc.miss;
				unicache.prefetchb->stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
				unicache.prefetchb->rtp_stats = unicache.prefetchb->stats_t;

				unicache.wbb->stats_t.readAc.access  += homenode_rtp_stats.writeAc.miss;
				unicache.wbb->stats_t.writeAc.access += homenode_rtp_stats.writeAc.miss;
			}
			unicache.wbb->rtp_stats = unicache.wbb->stats_t;

		}

	}

	unicache.power_t.reset();
	if (!((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory)))
	{
		//main refresh part
		/*
		double testdata = 0;
		 testdata	+= (unicache.caches->stats_t.readAc.hit*unicache.caches->local_result.power.readOp.dynamic+
				unicache.caches->stats_t.readAc.miss*unicache.caches->local_result.tag_array2->power.readOp.dynamic+
				unicache.caches->stats_t.writeAc.miss*unicache.caches->local_result.tag_array2->power.writeOp.dynamic+
				unicache.caches->stats_t.writeAc.access*unicache.caches->local_result.power.writeOp.dynamic);//write miss will also generate a write later
*/
		unicache.power_t.readOp.dynamic	+= ((unicache.caches->stats_t.readAc.hit-unicache.apcaches->stats_t.readAc.hit)*unicache.caches->local_result.power.readOp.dynamic+
				(unicache.caches->stats_t.readAc.miss-unicache.apcaches->stats_t.readAc.miss)*unicache.caches->local_result.tag_array2->power.readOp.dynamic+
				(unicache.caches->stats_t.writeAc.miss-unicache.apcaches->stats_t.writeAc.miss)*unicache.caches->local_result.tag_array2->power.writeOp.dynamic+
				(unicache.caches->stats_t.writeAc.access-unicache.apcaches->stats_t.writeAc.access)*unicache.caches->local_result.power.writeOp.dynamic);//write miss will also generate a write later
		unicache.power_t.readOp.dynamic	+= (unicache.apcaches->stats_t.readAc.hit*unicache.apcaches->local_result.power.readOp.dynamic+
				unicache.apcaches->stats_t.readAc.miss*unicache.caches->local_result.tag_array2->power.readOp.dynamic+		//caches: since tag array has normal vdd
				unicache.apcaches->stats_t.writeAc.miss*unicache.caches->local_result.tag_array2->power.writeOp.dynamic+	//caches: since tag array has normal vdd
				unicache.apcaches->stats_t.writeAc.access*unicache.apcaches->local_result.power.writeOp.dynamic);//write miss will also generate a write later
		//Raul
		if(cacheL == L2 && !is_tdp)
		{
/*		
			cout << "normal dynamic power: "		 << testdata << endl;
			cout << "with approximation: " << unicache.power_t.readOp.dynamic << endl;
			//local results
			cout << "unicache.caches->local_result.power.readOp.dynamic "  				<< unicache.caches->local_result.power.readOp.dynamic << endl;
			cout << "unicache.caches->local_result.tag_array2->power.readOp.dynamic " 	<< unicache.caches->local_result.tag_array2->power.readOp.dynamic << endl;
			cout << "unicache.caches->local_result.tag_array2->power.writeOp.dynamic "  << unicache.caches->local_result.tag_array2->power.writeOp.dynamic << endl;
			cout << "unicache.caches->local_result.power.writeOp.dynamic "  			<< unicache.caches->local_result.power.writeOp.dynamic << endl;
			cout << "unicache.apcaches->local_result.power.readOp.dynamic "  			<< unicache.apcaches->local_result.power.readOp.dynamic << endl;
			cout << "unicache.caches->local_result.tag_array2->power.readOp.dynamic "  	<< unicache.caches->local_result.tag_array2->power.readOp.dynamic << endl;
			cout << "unicache.caches->local_result.tag_array2->power.writeOp.dynamic "  << unicache.caches->local_result.tag_array2->power.writeOp.dynamic << endl;
			cout << "unicache.apcaches->local_result.power.writeOp.dynamic "  			<< unicache.apcaches->local_result.power.writeOp.dynamic << endl;
			//access counts
			cout << "unicache.caches->stats_t.readAc.hit "  			<< unicache.caches->stats_t.readAc.hit << endl;
			cout << "unicache.apcaches->stats_t.readAc.hit "  			<< unicache.apcaches->stats_t.readAc.hit << endl;
			cout << "unicache.caches->stats_t.readAc.miss "  			<< unicache.caches->stats_t.readAc.miss << endl;
			cout << "unicache.apcaches->stats_t.readAc.miss "  			<< unicache.apcaches->stats_t.readAc.miss << endl;
			cout << "unicache.caches->stats_t.writeAc.miss "  			<< unicache.caches->stats_t.writeAc.miss << endl;
			cout << "unicache.apcaches->stats_t.writeAc.miss "  			<< unicache.apcaches->stats_t.writeAc.miss << endl;
			cout << "unicache.caches->stats_t.writeAc.access "  			<< unicache.caches->stats_t.writeAc.access << endl;
			cout << "unicache.apcaches->stats_t.writeAc.access "  			<< unicache.apcaches->stats_t.writeAc.access << endl;
*/		
			
		}			//reasonable
		if (cachep.dir_ty==SBT)
		{
			unicache.power_t.readOp.dynamic	+= homenode_stats_t.readAc.hit * (unicache.caches->local_result.data_array2->power.readOp.dynamic*dir_overhead +
						unicache.caches->local_result.tag_array2->power.readOp.dynamic) +
					homenode_stats_t.readAc.miss*unicache.caches->local_result.tag_array2->power.readOp.dynamic +
					homenode_stats_t.writeAc.miss*unicache.caches->local_result.tag_array2->power.readOp.dynamic +
			        homenode_stats_t.writeAc.hit*(unicache.caches->local_result.data_array2->power.writeOp.dynamic*dir_overhead +
							unicache.caches->local_result.tag_array2->power.readOp.dynamic+
					homenode_stats_t.writeAc.miss*unicache.caches->local_result.power.writeOp.dynamic);//write miss on dynamic home node will generate a replacement write on whole cache block


		}

		unicache.power_t.readOp.dynamic	+=  unicache.missb->stats_t.readAc.access*unicache.missb->local_result.power.searchOp.dynamic +
		unicache.missb->stats_t.writeAc.access*unicache.missb->local_result.power.writeOp.dynamic;//each access to missb involves a CAM and a write
		unicache.power_t.readOp.dynamic	+=  unicache.ifb->stats_t.readAc.access*unicache.ifb->local_result.power.searchOp.dynamic +
		unicache.ifb->stats_t.writeAc.access*unicache.ifb->local_result.power.writeOp.dynamic;
		unicache.power_t.readOp.dynamic	+=  unicache.prefetchb->stats_t.readAc.access*unicache.prefetchb->local_result.power.searchOp.dynamic +
		unicache.prefetchb->stats_t.writeAc.access*unicache.prefetchb->local_result.power.writeOp.dynamic;
		unicache.power_t.readOp.dynamic	+=  unicache.wbb->stats_t.readAc.access*unicache.wbb->local_result.power.searchOp.dynamic +
		unicache.wbb->stats_t.writeAc.access*unicache.wbb->local_result.power.writeOp.dynamic;
	}
	else
	{
		unicache.power_t.readOp.dynamic	+= (unicache.caches->stats_t.readAc.access*unicache.caches->local_result.power.searchOp.dynamic+
				unicache.caches->stats_t.writeAc.access*unicache.caches->local_result.power.writeOp.dynamic);
	}
	
	if (is_tdp)
	{
		//Raul
		powerDef tmp;	tmp.reset();
		tmp.readOp.leakage = (unicache.apcaches->local_result.power.readOp.leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.leakage*(100-approx_percent))/100.0;
		tmp.readOp.gate_leakage = (unicache.apcaches->local_result.power.readOp.gate_leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.gate_leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.gate_leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.gate_leakage*(100-approx_percent))/100.0;
		tmp.readOp.power_gated_leakage = (unicache.apcaches->local_result.power.readOp.power_gated_leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.power_gated_leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.power_gated_leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.power_gated_leakage*(100-approx_percent))/100.0;	
		tmp.readOp.longer_channel_leakage = (unicache.apcaches->local_result.power.readOp.longer_channel_leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.longer_channel_leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.longer_channel_leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.longer_channel_leakage*(100-approx_percent))/100.0;								

/*							
		cout << "-- unicache.power (L2) & TDP == 1" << std::endl;
		cout << "leakage read " 			<< unicache.apcaches->local_result.power.readOp.leakage << endl;
		cout << "leakage write " 			<< unicache.apcaches->local_result.power.writeOp.leakage << endl;
		cout << "power gated leakage read " << unicache.apcaches->local_result.power.readOp.power_gated_leakage << endl;
		cout << "power gated leakage write "<< unicache.apcaches->local_result.power.writeOp.power_gated_leakage << endl;
		cout << "tmp.readOp.power_gated_leakage "	<< tmp.readOp.power_gated_leakage << endl;
		cout << "tmp.readOp.leakage "	<< tmp.readOp.leakage << endl; 	
*/		
		unicache.power = unicache.power_t + tmp;
		//cout << "-- unicache.power (L2) & TDP == 1" << std::endl;
		//cout << "dynamic: " << unicache.power.readOp.dynamic << endl;
		//cout << "approx leakage: " << unicache.power.readOp.leakage << endl;
		//cout << "approx gate leakage: " << unicache.power.readOp.gate_leakage << endl;		
		//cout << "approx power gated leakage: " << unicache.power.readOp.power_gated_leakage << endl;	
		//unicache.rt_power = unicache.power_t + (unicache.caches->local_result.power)*pppm_lkg;
		if (!((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory)))
		{
			unicache.power = unicache.power+
			(unicache.missb->local_result.power +
					unicache.ifb->local_result.power +
					unicache.prefetchb->local_result.power +
					unicache.wbb->local_result.power)*pppm_lkg;
		}
		//cout << "power_gated_leakage before: "  << power.readOp.power_gated_leakage << endl;
		power     = power + unicache.power;
		//cout << "power_gated_leakage after: "  << power.readOp.power_gated_leakage << endl;
//		cout<<"unicache.caches->local_result.power.readOp.dynamic"<<unicache.caches->local_result.power.readOp.dynamic<<endl;
//		cout<<"unicache.caches->local_result.power.writeOp.dynamic"<<unicache.caches->local_result.power.writeOp.dynamic<<endl;
	}
	else
	{
		powerDef tmp;	tmp.reset();
		tmp.readOp.leakage = (unicache.apcaches->local_result.power.readOp.leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.leakage*(100-approx_percent))/100.0;
		tmp.readOp.gate_leakage = (unicache.apcaches->local_result.power.readOp.gate_leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.gate_leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.gate_leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.gate_leakage*(100-approx_percent))/100.0;
		tmp.readOp.power_gated_leakage = (unicache.apcaches->local_result.power.readOp.power_gated_leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.power_gated_leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.power_gated_leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.power_gated_leakage*(100-approx_percent))/100.0;		
		tmp.readOp.longer_channel_leakage = (unicache.apcaches->local_result.power.readOp.longer_channel_leakage*approx_percent + 
							unicache.caches->local_result.power.readOp.longer_channel_leakage*(100-approx_percent) +
							unicache.apcaches->local_result.power.writeOp.longer_channel_leakage*approx_percent +
							unicache.caches->local_result.power.writeOp.longer_channel_leakage*(100-approx_percent))/100.0;								
/*
		cout << "gate leakage read " 		<< unicache.apcaches->local_result.power.readOp.gate_leakage << endl;
		cout << "gate leakage write " 		<< unicache.apcaches->local_result.power.writeOp.gate_leakage << endl;
		cout << "-- unicache.power (L2) & TDP == 0" << std::endl;
		cout << "leakage read " 			<< unicache.apcaches->local_result.power.readOp.leakage << endl;
		cout << "leakage write " 			<< unicache.apcaches->local_result.power.writeOp.leakage << endl;
		cout << "power gated leakage read " << unicache.apcaches->local_result.power.readOp.power_gated_leakage << endl;
		cout << "power gated leakage write "<< unicache.apcaches->local_result.power.writeOp.power_gated_leakage << endl;
		cout << "tmp.readOp.power_gated_leakage "	<< tmp.readOp.power_gated_leakage << endl;
		cout << "tmp.readOp.leakage "	<< tmp.readOp.leakage << endl;  
*/
		//unicache.rt_power = unicache.power_t + (unicache.caches->local_result.power)*pppm_lkg;
		unicache.rt_power = unicache.power_t + tmp;
/*		
		cout << "-- unicache.power (L2) & TDP == 0" << endl;
		cout << "dynamic: " << unicache.rt_power.readOp.dynamic << endl;
		cout << "approx leakage: " << unicache.rt_power.readOp.leakage << endl;
		cout << "approx gate leakage: " << unicache.rt_power.readOp.gate_leakage << endl;			//reasonable
		cout << "approx power gated leakage: " << unicache.rt_power.readOp.power_gated_leakage << endl;
*/
		if (!((cachep.dir_ty==ST&& cacheL==L1Directory)||(cachep.dir_ty==ST&& cacheL==L2Directory)))
		{
			(unicache.rt_power = unicache.rt_power +
					unicache.missb->local_result.power +
					unicache.ifb->local_result.power +
					unicache.prefetchb->local_result.power +
					unicache.wbb->local_result.power)*pppm_lkg;
		}
		rt_power     = rt_power + unicache.rt_power; 
		//cout  << "_-----------------------_" << endl;
	}
}

void SharedCache::displayEnergy(uint32_t indent,bool is_tdp)
{
	string indent_str(indent, ' ');
	string indent_str_next(indent+2, ' ');
	bool long_channel = XML->sys.longer_channel_device;
	bool power_gating = XML->sys.power_gating;

	if (is_tdp)
	{
		cout << (XML->sys.Private_L2? indent_str:"")<< cachep.name << endl;
		cout << indent_str << "Area = " << area.get_area()*1e-6<< " mm^2" << endl;
		cout << indent_str << "Peak Dynamic = " << power.readOp.dynamic*cachep.clockRate << " W" << endl;
		cout << indent_str << "Subthreshold Leakage = "
			<< (long_channel? power.readOp.longer_channel_leakage:power.readOp.leakage) <<" W" << endl;
		if (power_gating) cout << indent_str << "Subthreshold Leakage with power gating = "
						<< (power.readOp.power_gated_leakage * (long_channel? power.readOp.longer_channel_leakage/power.readOp.leakage:1) )  << " W" << endl;
		cout << indent_str << "Gate Leakage = " << power.readOp.gate_leakage << " W" << endl;
		cout << indent_str << "Runtime Dynamic = " << rt_power.readOp.dynamic/cachep.executionTime << " W" << endl;
		cout <<endl;
	}
	else
	{		//Raul
		cout << indent_str << "Runtime Dynamic = " << rt_power.readOp.dynamic/cachep.executionTime << " W" << endl;
		cout << indent_str << "Subthreshold Leakage = " << rt_power.readOp.leakage <<" W" << endl;
		cout << indent_str_next << " Gate Leakage = " << rt_power.readOp.gate_leakage << " W" << endl;
	}
}



void SharedCache::set_cache_param()
{
	//interface_ip.approx_mode = 0;		//Raul
	if (cacheL==L2)
	{
		cachep.name = "L2";
		cachep.clockRate       = XML->sys.L2[ithCache].clockrate;
		cachep.clockRate       *= 1e6;
		cachep.executionTime = XML->sys.total_cycles/(XML->sys.target_core_clockrate*1e6);
		interface_ip.data_arr_ram_cell_tech_type    = XML->sys.L2[ithCache].device_type;//long channel device LSTP
		interface_ip.data_arr_peri_global_tech_type = XML->sys.L2[ithCache].device_type;
		interface_ip.tag_arr_ram_cell_tech_type     = XML->sys.L2[ithCache].device_type;
		interface_ip.tag_arr_peri_global_tech_type  = XML->sys.L2[ithCache].device_type;
		cachep.capacity      = XML->sys.L2[ithCache].L2_config[0];
		cachep.blockW        = XML->sys.L2[ithCache].L2_config[1];
		cachep.assoc         = XML->sys.L2[ithCache].L2_config[2];
		cachep.nbanks        = XML->sys.L2[ithCache].L2_config[3];
		cachep.throughput    = XML->sys.L2[ithCache].L2_config[4]/cachep.clockRate;
		cachep.latency       = XML->sys.L2[ithCache].L2_config[5]/cachep.clockRate;
		cachep.missb_size    = XML->sys.L2[ithCache].buffer_sizes[0];
		cachep.fu_size       = XML->sys.L2[ithCache].buffer_sizes[1];
		cachep.prefetchb_size= XML->sys.L2[ithCache].buffer_sizes[2];
		cachep.wbb_size      = XML->sys.L2[ithCache].buffer_sizes[3];
		cachep.duty_cycle    = XML->sys.L2[ithCache].duty_cycle;
		if (!XML->sys.L2[ithCache].merged_dir)
		{
			cachep.dir_ty = NonDir;
		}
		else
		{
			cachep.dir_ty = SBT;
			cachep.dir_duty_cycle  = XML->sys.L2[ithCache].dir_duty_cycle;
		}

		if (XML->sys.Private_L2 && XML->sys.core[ithCache].vdd>0)
		{
		  interface_ip.specific_hp_vdd = true;
		  interface_ip.specific_lop_vdd = true;
		  interface_ip.specific_lstp_vdd = true;
		  interface_ip.hp_Vdd   = XML->sys.core[ithCache].vdd;
		  interface_ip.lop_Vdd  = XML->sys.core[ithCache].vdd;
		  interface_ip.lstp_Vdd = XML->sys.core[ithCache].vdd;
		}
		if (!XML->sys.Private_L2 && XML->sys.L2[ithCache].vdd>0)
		{
			interface_ip.specific_hp_vdd = true;
			interface_ip.specific_lop_vdd = true;
			interface_ip.specific_lstp_vdd = true;
			interface_ip.hp_Vdd   = XML->sys.L2[ithCache].vdd;
			interface_ip.lop_Vdd  = XML->sys.L2[ithCache].vdd;
			interface_ip.lstp_Vdd = XML->sys.L2[ithCache].vdd;
		}
	}
	else if (cacheL==L3)
	{
		cachep.name = "L3";
		cachep.clockRate       = XML->sys.L3[ithCache].clockrate;
		cachep.clockRate       *= 1e6;
		cachep.executionTime   = XML->sys.total_cycles/(XML->sys.target_core_clockrate*1e6);
		interface_ip.data_arr_ram_cell_tech_type    = XML->sys.L3[ithCache].device_type;//long channel device LSTP
		interface_ip.data_arr_peri_global_tech_type = XML->sys.L3[ithCache].device_type;
		interface_ip.tag_arr_ram_cell_tech_type     = XML->sys.L3[ithCache].device_type;
		interface_ip.tag_arr_peri_global_tech_type  = XML->sys.L3[ithCache].device_type;
		cachep.capacity      = XML->sys.L3[ithCache].L3_config[0];
		cachep.blockW        = XML->sys.L3[ithCache].L3_config[1];
		cachep.assoc         = XML->sys.L3[ithCache].L3_config[2];
		cachep.nbanks        = XML->sys.L3[ithCache].L3_config[3];
		cachep.throughput    = XML->sys.L3[ithCache].L3_config[4]/cachep.clockRate;
		cachep.latency       = XML->sys.L3[ithCache].L3_config[5]/cachep.clockRate;
		cachep.missb_size    = XML->sys.L3[ithCache].buffer_sizes[0];
		cachep.fu_size       = XML->sys.L3[ithCache].buffer_sizes[1];
		cachep.prefetchb_size= XML->sys.L3[ithCache].buffer_sizes[2];
		cachep.wbb_size      = XML->sys.L3[ithCache].buffer_sizes[3];
		cachep.duty_cycle    = XML->sys.L3[ithCache].duty_cycle;
		if (!XML->sys.L2[ithCache].merged_dir)
		{
			cachep.dir_ty = NonDir;
		}
		else
		{
			cachep.dir_ty = SBT;
			cachep.dir_duty_cycle  = XML->sys.L2[ithCache].dir_duty_cycle;
		}
		if ( XML->sys.L3[ithCache].vdd>0)
		{
			interface_ip.specific_hp_vdd = true;
			interface_ip.specific_lop_vdd = true;
			interface_ip.specific_lstp_vdd = true;
			interface_ip.hp_Vdd   = XML->sys.L3[ithCache].vdd;
			interface_ip.lop_Vdd  = XML->sys.L3[ithCache].vdd;
			interface_ip.lstp_Vdd = XML->sys.L3[ithCache].vdd;
		}
	}
	else if (cacheL==L1Directory)
		{
			cachep.name = "First Level Directory";
			cachep.dir_ty = (enum Dir_type) XML->sys.L1Directory[ithCache].Directory_type;
			cachep.clockRate       = XML->sys.L1Directory[ithCache].clockrate;
			cachep.clockRate       *= 1e6;
			cachep.executionTime   = XML->sys.total_cycles/(XML->sys.target_core_clockrate*1e6);
			interface_ip.data_arr_ram_cell_tech_type    = XML->sys.L1Directory[ithCache].device_type;//long channel device LSTP
			interface_ip.data_arr_peri_global_tech_type = XML->sys.L1Directory[ithCache].device_type;
			interface_ip.tag_arr_ram_cell_tech_type     = XML->sys.L1Directory[ithCache].device_type;
			interface_ip.tag_arr_peri_global_tech_type  = XML->sys.L1Directory[ithCache].device_type;
			cachep.capacity      = XML->sys.L1Directory[ithCache].Dir_config[0];
			cachep.blockW        = XML->sys.L1Directory[ithCache].Dir_config[1];
			cachep.assoc         = XML->sys.L1Directory[ithCache].Dir_config[2];
			cachep.nbanks        = XML->sys.L1Directory[ithCache].Dir_config[3];
			cachep.throughput    = XML->sys.L1Directory[ithCache].Dir_config[4]/cachep.clockRate;
			cachep.latency       = XML->sys.L1Directory[ithCache].Dir_config[5]/cachep.clockRate;
			cachep.missb_size    = XML->sys.L1Directory[ithCache].buffer_sizes[0];
			cachep.fu_size       = XML->sys.L1Directory[ithCache].buffer_sizes[1];
			cachep.prefetchb_size= XML->sys.L1Directory[ithCache].buffer_sizes[2];
			cachep.wbb_size      = XML->sys.L1Directory[ithCache].buffer_sizes[3];
			cachep.duty_cycle    = XML->sys.L1Directory[ithCache].duty_cycle;

			if ( XML->sys.L1Directory[ithCache].vdd>0)
			{
				interface_ip.specific_hp_vdd = true;
				interface_ip.specific_lop_vdd = true;
				interface_ip.specific_lstp_vdd = true;
				interface_ip.hp_Vdd   = XML->sys.L1Directory[ithCache].vdd;
				interface_ip.lop_Vdd  = XML->sys.L1Directory[ithCache].vdd;
				interface_ip.lstp_Vdd = XML->sys.L1Directory[ithCache].vdd;
			}
		}
	else if (cacheL==L2Directory)
		{
			cachep.name = "Second Level Directory";
			cachep.dir_ty = (enum Dir_type) XML->sys.L2Directory[ithCache].Directory_type;
			cachep.clockRate       = XML->sys.L2Directory[ithCache].clockrate;
			cachep.clockRate       *= 1e6;
			cachep.executionTime   = XML->sys.total_cycles/(XML->sys.target_core_clockrate*1e6);
			interface_ip.data_arr_ram_cell_tech_type    = XML->sys.L2Directory[ithCache].device_type;//long channel device LSTP
			interface_ip.data_arr_peri_global_tech_type = XML->sys.L2Directory[ithCache].device_type;
			interface_ip.tag_arr_ram_cell_tech_type     = XML->sys.L2Directory[ithCache].device_type;
			interface_ip.tag_arr_peri_global_tech_type  = XML->sys.L2Directory[ithCache].device_type;
			cachep.capacity      = XML->sys.L2Directory[ithCache].Dir_config[0];
			cachep.blockW        = XML->sys.L2Directory[ithCache].Dir_config[1];
			cachep.assoc         = XML->sys.L2Directory[ithCache].Dir_config[2];
			cachep.nbanks        = XML->sys.L2Directory[ithCache].Dir_config[3];
			cachep.throughput    = XML->sys.L2Directory[ithCache].Dir_config[4]/cachep.clockRate;
			cachep.latency       = XML->sys.L2Directory[ithCache].Dir_config[5]/cachep.clockRate;
			cachep.missb_size    = XML->sys.L2Directory[ithCache].buffer_sizes[0];
			cachep.fu_size       = XML->sys.L2Directory[ithCache].buffer_sizes[1];
			cachep.prefetchb_size= XML->sys.L2Directory[ithCache].buffer_sizes[2];
			cachep.wbb_size      = XML->sys.L2Directory[ithCache].buffer_sizes[3];
			cachep.duty_cycle    = XML->sys.L2Directory[ithCache].duty_cycle;

			if ( XML->sys.L2Directory[ithCache].vdd>0)
			{
				interface_ip.specific_hp_vdd = true;
				interface_ip.specific_lop_vdd = true;
				interface_ip.specific_lstp_vdd = true;
				interface_ip.hp_Vdd   = XML->sys.L2Directory[ithCache].vdd;
				interface_ip.lop_Vdd  = XML->sys.L2Directory[ithCache].vdd;
				interface_ip.lstp_Vdd = XML->sys.L2Directory[ithCache].vdd;
			}
		}
	//cachep.cache_duty_cycle=cachep.dir_duty_cycle = 0.35;


}

