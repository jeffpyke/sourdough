#include <iostream>
#include <algorithm>

#include "controller.hh"
#include "timestamp.hh"


using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), rtts()
{
}

/* Get current window size, in datagrams */
unsigned int Controller::window_size()
{
  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp,
                                    /* in milliseconds */
				    const bool after_timeout
				    /* datagram was sent because of a timeout */ )
{
  /* Default: take no action */
  // if (after_timeout) {
  //   the_window_size = max(the_window_size*0.5, 1.0);
  // }
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << " (timeout = " << after_timeout << ")\n";
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */
  the_window_size = the_window_size + 1 / (the_window_size);
  uint64_t rtt = timestamp_ack_received - send_timestamp_acked;
  avg_rtt = avg_rtt * 0.5 + rtt * 0.5;
  
  if (rtts.size() > 50) {
    rtts.pop_back();
  }
  rtts.insert(rtts.begin(), rtt);
  //min_rtt = *min_element(rtts.begin(), rtts.end());
  min_rtt = min(rtt, min_rtt);
  if (rtt > 2 * min_rtt && timestamp_ack_received - last_shrink > 0.5*min_rtt) {
    last_shrink = timestamp_ack_received;
    the_window_size = max(the_window_size*(7/8.0), 1.0);
  }
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)" << "avg rtt " << avg_rtt << "min rtt" << min_rtt
	 << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms()
{
  return 1.5*avg_rtt;
}
