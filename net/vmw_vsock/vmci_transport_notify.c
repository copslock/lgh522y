/*
 * VMware vSockets Driver
 *
 * Copyright (C) 2009-2013 VMware, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation version 2 and no later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include <linux/types.h>
#include <linux/socket.h>
#include <linux/stddef.h>
#include <net/sock.h>

#include "vmci_transport_notify.h"

#define PKT_FIELD(vsk, field_name) (vmci_trans(vsk)->notify.pkt.field_name)

static bool vmci_transport_notify_waiting_write(struct vsock_sock *vsk)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	bool retval;
	u64 notify_limit;

	if (!PKT_FIELD(vsk, peer_waiting_write))
		return false;

#ifdef VSOCK_OPTIMIZATION_FLOW_CONTROL
	/*                                                                  
                                                                        
                                                                   
                                                                        
                                                                
  */

	if (!PKT_FIELD(vsk, peer_waiting_write_detected)) {
		PKT_FIELD(vsk, peer_waiting_write_detected) = true;
		if (PKT_FIELD(vsk, write_notify_window) < PAGE_SIZE) {
			PKT_FIELD(vsk, write_notify_window) =
			    PKT_FIELD(vsk, write_notify_min_window);
		} else {
			PKT_FIELD(vsk, write_notify_window) -= PAGE_SIZE;
			if (PKT_FIELD(vsk, write_notify_window) <
			    PKT_FIELD(vsk, write_notify_min_window))
				PKT_FIELD(vsk, write_notify_window) =
				    PKT_FIELD(vsk, write_notify_min_window);

		}
	}
	notify_limit = vmci_trans(vsk)->consume_size -
		PKT_FIELD(vsk, write_notify_window);
#else
	notify_limit = 0;
#endif

	/*                                                                
                                                                      
                                                                      
                                                                      
             
   
                                                                     
                                                                    
                                                          
                                                                       
                                                                     
                                                              
                                            
  */
	retval = vmci_qpair_consume_free_space(vmci_trans(vsk)->qpair) >
		notify_limit;
#ifdef VSOCK_OPTIMIZATION_FLOW_CONTROL
	if (retval) {
		/*
                                                               
                                                              
   */

		PKT_FIELD(vsk, peer_waiting_write_detected) = false;
	}
#endif
	return retval;
#else
	return true;
#endif
}

static bool vmci_transport_notify_waiting_read(struct vsock_sock *vsk)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	if (!PKT_FIELD(vsk, peer_waiting_read))
		return false;

	/*                                                                    
                                                                       
                                                                       
                                                               
             
  */
	return vmci_qpair_produce_buf_ready(vmci_trans(vsk)->qpair) > 0;
#else
	return true;
#endif
}

static void
vmci_transport_handle_waiting_read(struct sock *sk,
				   struct vmci_transport_packet *pkt,
				   bool bottom_half,
				   struct sockaddr_vm *dst,
				   struct sockaddr_vm *src)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	struct vsock_sock *vsk;

	vsk = vsock_sk(sk);

	PKT_FIELD(vsk, peer_waiting_read) = true;
	memcpy(&PKT_FIELD(vsk, peer_waiting_read_info), &pkt->u.wait,
	       sizeof(PKT_FIELD(vsk, peer_waiting_read_info)));

	if (vmci_transport_notify_waiting_read(vsk)) {
		bool sent;

		if (bottom_half)
			sent = vmci_transport_send_wrote_bh(dst, src) > 0;
		else
			sent = vmci_transport_send_wrote(sk) > 0;

		if (sent)
			PKT_FIELD(vsk, peer_waiting_read) = false;
	}
#endif
}

static void
vmci_transport_handle_waiting_write(struct sock *sk,
				    struct vmci_transport_packet *pkt,
				    bool bottom_half,
				    struct sockaddr_vm *dst,
				    struct sockaddr_vm *src)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	struct vsock_sock *vsk;

	vsk = vsock_sk(sk);

	PKT_FIELD(vsk, peer_waiting_write) = true;
	memcpy(&PKT_FIELD(vsk, peer_waiting_write_info), &pkt->u.wait,
	       sizeof(PKT_FIELD(vsk, peer_waiting_write_info)));

	if (vmci_transport_notify_waiting_write(vsk)) {
		bool sent;

		if (bottom_half)
			sent = vmci_transport_send_read_bh(dst, src) > 0;
		else
			sent = vmci_transport_send_read(sk) > 0;

		if (sent)
			PKT_FIELD(vsk, peer_waiting_write) = false;
	}
#endif
}

static void
vmci_transport_handle_read(struct sock *sk,
			   struct vmci_transport_packet *pkt,
			   bool bottom_half,
			   struct sockaddr_vm *dst, struct sockaddr_vm *src)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	struct vsock_sock *vsk;

	vsk = vsock_sk(sk);
	PKT_FIELD(vsk, sent_waiting_write) = false;
#endif

	sk->sk_write_space(sk);
}

static bool send_waiting_read(struct sock *sk, u64 room_needed)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	struct vsock_sock *vsk;
	struct vmci_transport_waiting_info waiting_info;
	u64 tail;
	u64 head;
	u64 room_left;
	bool ret;

	vsk = vsock_sk(sk);

	if (PKT_FIELD(vsk, sent_waiting_read))
		return true;

	if (PKT_FIELD(vsk, write_notify_window) <
			vmci_trans(vsk)->consume_size)
		PKT_FIELD(vsk, write_notify_window) =
		    min(PKT_FIELD(vsk, write_notify_window) + PAGE_SIZE,
			vmci_trans(vsk)->consume_size);

	vmci_qpair_get_consume_indexes(vmci_trans(vsk)->qpair, &tail, &head);
	room_left = vmci_trans(vsk)->consume_size - head;
	if (room_needed >= room_left) {
		waiting_info.offset = room_needed - room_left;
		waiting_info.generation =
		    PKT_FIELD(vsk, consume_q_generation) + 1;
	} else {
		waiting_info.offset = head + room_needed;
		waiting_info.generation = PKT_FIELD(vsk, consume_q_generation);
	}

	ret = vmci_transport_send_waiting_read(sk, &waiting_info) > 0;
	if (ret)
		PKT_FIELD(vsk, sent_waiting_read) = true;

	return ret;
#else
	return true;
#endif
}

static bool send_waiting_write(struct sock *sk, u64 room_needed)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	struct vsock_sock *vsk;
	struct vmci_transport_waiting_info waiting_info;
	u64 tail;
	u64 head;
	u64 room_left;
	bool ret;

	vsk = vsock_sk(sk);

	if (PKT_FIELD(vsk, sent_waiting_write))
		return true;

	vmci_qpair_get_produce_indexes(vmci_trans(vsk)->qpair, &tail, &head);
	room_left = vmci_trans(vsk)->produce_size - tail;
	if (room_needed + 1 >= room_left) {
		/*                                     */
		waiting_info.offset = room_needed + 1 - room_left;
		waiting_info.generation = PKT_FIELD(vsk, produce_q_generation);
	} else {
		waiting_info.offset = tail + room_needed + 1;
		waiting_info.generation =
		    PKT_FIELD(vsk, produce_q_generation) - 1;
	}

	ret = vmci_transport_send_waiting_write(sk, &waiting_info) > 0;
	if (ret)
		PKT_FIELD(vsk, sent_waiting_write) = true;

	return ret;
#else
	return true;
#endif
}

static int vmci_transport_send_read_notification(struct sock *sk)
{
	struct vsock_sock *vsk;
	bool sent_read;
	unsigned int retries;
	int err;

	vsk = vsock_sk(sk);
	sent_read = false;
	retries = 0;
	err = 0;

	if (vmci_transport_notify_waiting_write(vsk)) {
		/*                                                        
                                                              
                                                             
                                                             
                                                                
                       
   */
		while (!(vsk->peer_shutdown & RCV_SHUTDOWN) &&
		       !sent_read &&
		       retries < VMCI_TRANSPORT_MAX_DGRAM_RESENDS) {
			err = vmci_transport_send_read(sk);
			if (err >= 0)
				sent_read = true;

			retries++;
		}

		if (retries >= VMCI_TRANSPORT_MAX_DGRAM_RESENDS)
			pr_err("%p unable to send read notify to peer\n", sk);
		else
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
			PKT_FIELD(vsk, peer_waiting_write) = false;
#endif

	}
	return err;
}

static void
vmci_transport_handle_wrote(struct sock *sk,
			    struct vmci_transport_packet *pkt,
			    bool bottom_half,
			    struct sockaddr_vm *dst, struct sockaddr_vm *src)
{
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	struct vsock_sock *vsk = vsock_sk(sk);
	PKT_FIELD(vsk, sent_waiting_read) = false;
#endif
	sk->sk_data_ready(sk, 0);
}

static void vmci_transport_notify_pkt_socket_init(struct sock *sk)
{
	struct vsock_sock *vsk = vsock_sk(sk);

	PKT_FIELD(vsk, write_notify_window) = PAGE_SIZE;
	PKT_FIELD(vsk, write_notify_min_window) = PAGE_SIZE;
	PKT_FIELD(vsk, peer_waiting_read) = false;
	PKT_FIELD(vsk, peer_waiting_write) = false;
	PKT_FIELD(vsk, peer_waiting_write_detected) = false;
	PKT_FIELD(vsk, sent_waiting_read) = false;
	PKT_FIELD(vsk, sent_waiting_write) = false;
	PKT_FIELD(vsk, produce_q_generation) = 0;
	PKT_FIELD(vsk, consume_q_generation) = 0;

	memset(&PKT_FIELD(vsk, peer_waiting_read_info), 0,
	       sizeof(PKT_FIELD(vsk, peer_waiting_read_info)));
	memset(&PKT_FIELD(vsk, peer_waiting_write_info), 0,
	       sizeof(PKT_FIELD(vsk, peer_waiting_write_info)));
}

static void vmci_transport_notify_pkt_socket_destruct(struct vsock_sock *vsk)
{
}

static int
vmci_transport_notify_pkt_poll_in(struct sock *sk,
				  size_t target, bool *data_ready_now)
{
	struct vsock_sock *vsk = vsock_sk(sk);

	if (vsock_stream_has_data(vsk)) {
		*data_ready_now = true;
	} else {
		/*                                                        
                                                            
          
   */
		if (sk->sk_state == SS_CONNECTED) {
			if (!send_waiting_read(sk, 1))
				return -1;

		}
		*data_ready_now = false;
	}

	return 0;
}

static int
vmci_transport_notify_pkt_poll_out(struct sock *sk,
				   size_t target, bool *space_avail_now)
{
	s64 produce_q_free_space;
	struct vsock_sock *vsk = vsock_sk(sk);

	produce_q_free_space = vsock_stream_has_space(vsk);
	if (produce_q_free_space > 0) {
		*space_avail_now = true;
		return 0;
	} else if (produce_q_free_space == 0) {
		/*                                                             
                                                                 
                                                           
                                                            
                                                              
                                                              
              
   */
		if (!send_waiting_write(sk, 1))
			return -1;

		*space_avail_now = false;
	}

	return 0;
}

static int
vmci_transport_notify_pkt_recv_init(
			struct sock *sk,
			size_t target,
			struct vmci_transport_recv_notify_data *data)
{
	struct vsock_sock *vsk = vsock_sk(sk);

#ifdef VSOCK_OPTIMIZATION_WAITING_NOTIFY
	data->consume_head = 0;
	data->produce_tail = 0;
#ifdef VSOCK_OPTIMIZATION_FLOW_CONTROL
	data->notify_on_block = false;

	if (PKT_FIELD(vsk, write_notify_min_window) < target + 1) {
		PKT_FIELD(vsk, write_notify_min_window) = target + 1;
		if (PKT_FIELD(vsk, write_notify_window) <
		    PKT_FIELD(vsk, write_notify_min_window)) {
			/*                                              
                                                        
                                                          
                                                       
                                                        
    */

			PKT_FIELD(vsk, write_notify_window) =
			    PKT_FIELD(vsk, write_notify_min_window);
			data->notify_on_block = true;
		}
	}
#endif
#endif

	return 0;
}

static int
vmci_transport_notify_pkt_recv_pre_block(
				struct sock *sk,
				size_t target,
				struct vmci_transport_recv_notify_data *data)
{
	int err = 0;

	/*                                                       */
	if (!send_waiting_read(sk, target)) {
		err = -EHOSTUNREACH;
		return err;
	}
#ifdef VSOCK_OPTIMIZATION_FLOW_CONTROL
	if (data->notify_on_block) {
		err = vmci_transport_send_read_notification(sk);
		if (err < 0)
			return err;

		data->notify_on_block = false;
	}
#endif

	return err;
}

static int
vmci_transport_notify_pkt_recv_pre_dequeue(
				struct sock *sk,
				size_t target,
				struct vmci_transport_recv_notify_data *data)
{
	struct vsock_sock *vsk = vsock_sk(sk);

	/*                                                                     
                                                          
  */
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	vmci_qpair_get_consume_indexes(vmci_trans(vsk)->qpair,
				       &data->produce_tail,
				       &data->consume_head);
#endif

	return 0;
}

static int
vmci_transport_notify_pkt_recv_post_dequeue(
				struct sock *sk,
				size_t target,
				ssize_t copied,
				bool data_read,
				struct vmci_transport_recv_notify_data *data)
{
	struct vsock_sock *vsk;
	int err;

	vsk = vsock_sk(sk);
	err = 0;

	if (data_read) {
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
		/*                                                         
                                                               
                               
   */
		if (copied >=
			vmci_trans(vsk)->consume_size - data->consume_head)
			PKT_FIELD(vsk, consume_q_generation)++;
#endif

		err = vmci_transport_send_read_notification(sk);
		if (err < 0)
			return err;

	}
	return err;
}

static int
vmci_transport_notify_pkt_send_init(
			struct sock *sk,
			struct vmci_transport_send_notify_data *data)
{
#ifdef VSOCK_OPTIMIZATION_WAITING_NOTIFY
	data->consume_head = 0;
	data->produce_tail = 0;
#endif

	return 0;
}

static int
vmci_transport_notify_pkt_send_pre_block(
				struct sock *sk,
				struct vmci_transport_send_notify_data *data)
{
	/*                                                        */
	if (!send_waiting_write(sk, 1))
		return -EHOSTUNREACH;

	return 0;
}

static int
vmci_transport_notify_pkt_send_pre_enqueue(
				struct sock *sk,
				struct vmci_transport_send_notify_data *data)
{
	struct vsock_sock *vsk = vsock_sk(sk);

#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	vmci_qpair_get_produce_indexes(vmci_trans(vsk)->qpair,
				       &data->produce_tail,
				       &data->consume_head);
#endif

	return 0;
}

static int
vmci_transport_notify_pkt_send_post_enqueue(
				struct sock *sk,
				ssize_t written,
				struct vmci_transport_send_notify_data *data)
{
	int err = 0;
	struct vsock_sock *vsk;
	bool sent_wrote = false;
	int retries = 0;

	vsk = vsock_sk(sk);

#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
	/*                                                                   
                                                                   
               
  */
	if (written >= vmci_trans(vsk)->produce_size - data->produce_tail)
		PKT_FIELD(vsk, produce_q_generation)++;

#endif

	if (vmci_transport_notify_waiting_read(vsk)) {
		/*                                                           
                                                                 
                                                                 
                  
   */
		while (!(vsk->peer_shutdown & RCV_SHUTDOWN) &&
		       !sent_wrote &&
		       retries < VMCI_TRANSPORT_MAX_DGRAM_RESENDS) {
			err = vmci_transport_send_wrote(sk);
			if (err >= 0)
				sent_wrote = true;

			retries++;
		}

		if (retries >= VMCI_TRANSPORT_MAX_DGRAM_RESENDS) {
			pr_err("%p unable to send wrote notify to peer\n", sk);
			return err;
		} else {
#if defined(VSOCK_OPTIMIZATION_WAITING_NOTIFY)
			PKT_FIELD(vsk, peer_waiting_read) = false;
#endif
		}
	}
	return err;
}

static void
vmci_transport_notify_pkt_handle_pkt(
			struct sock *sk,
			struct vmci_transport_packet *pkt,
			bool bottom_half,
			struct sockaddr_vm *dst,
			struct sockaddr_vm *src, bool *pkt_processed)
{
	bool processed = false;

	switch (pkt->type) {
	case VMCI_TRANSPORT_PACKET_TYPE_WROTE:
		vmci_transport_handle_wrote(sk, pkt, bottom_half, dst, src);
		processed = true;
		break;
	case VMCI_TRANSPORT_PACKET_TYPE_READ:
		vmci_transport_handle_read(sk, pkt, bottom_half, dst, src);
		processed = true;
		break;
	case VMCI_TRANSPORT_PACKET_TYPE_WAITING_WRITE:
		vmci_transport_handle_waiting_write(sk, pkt, bottom_half,
						    dst, src);
		processed = true;
		break;

	case VMCI_TRANSPORT_PACKET_TYPE_WAITING_READ:
		vmci_transport_handle_waiting_read(sk, pkt, bottom_half,
						   dst, src);
		processed = true;
		break;
	}

	if (pkt_processed)
		*pkt_processed = processed;
}

static void vmci_transport_notify_pkt_process_request(struct sock *sk)
{
	struct vsock_sock *vsk = vsock_sk(sk);

	PKT_FIELD(vsk, write_notify_window) = vmci_trans(vsk)->consume_size;
	if (vmci_trans(vsk)->consume_size <
		PKT_FIELD(vsk, write_notify_min_window))
		PKT_FIELD(vsk, write_notify_min_window) =
			vmci_trans(vsk)->consume_size;
}

static void vmci_transport_notify_pkt_process_negotiate(struct sock *sk)
{
	struct vsock_sock *vsk = vsock_sk(sk);

	PKT_FIELD(vsk, write_notify_window) = vmci_trans(vsk)->consume_size;
	if (vmci_trans(vsk)->consume_size <
		PKT_FIELD(vsk, write_notify_min_window))
		PKT_FIELD(vsk, write_notify_min_window) =
			vmci_trans(vsk)->consume_size;
}

/*                                         */
struct vmci_transport_notify_ops vmci_transport_notify_pkt_ops = {
	vmci_transport_notify_pkt_socket_init,
	vmci_transport_notify_pkt_socket_destruct,
	vmci_transport_notify_pkt_poll_in,
	vmci_transport_notify_pkt_poll_out,
	vmci_transport_notify_pkt_handle_pkt,
	vmci_transport_notify_pkt_recv_init,
	vmci_transport_notify_pkt_recv_pre_block,
	vmci_transport_notify_pkt_recv_pre_dequeue,
	vmci_transport_notify_pkt_recv_post_dequeue,
	vmci_transport_notify_pkt_send_init,
	vmci_transport_notify_pkt_send_pre_block,
	vmci_transport_notify_pkt_send_pre_enqueue,
	vmci_transport_notify_pkt_send_post_enqueue,
	vmci_transport_notify_pkt_process_request,
	vmci_transport_notify_pkt_process_negotiate,
};
