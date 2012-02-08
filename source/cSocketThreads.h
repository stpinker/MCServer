
// cSocketThreads.h

// Interfaces to the cSocketThreads class representing the heart of MCS's client networking.
// This object takes care of network communication, groups sockets into threads and uses as little threads as possible for full read / write support
// For more detail, see http://forum.mc-server.org/showthread.php?tid=327





/// How many clients should one thread handle? (must be less than FD_SETSIZE - 1 for your platform)
#define MAX_SLOTS 63





#pragma once
#ifndef CSOCKETTHREADS_H_INCLUDED
#define CSOCKETTHREADS_H_INCLUDED

#include "packets/cPacket.h"
#include "cIsThread.h"




// Check MAX_SLOTS:
#if MAX_SLOTS >= FD_SETSIZE
	#error "MAX_SLOTS must be less than FD_SETSIZE - 1 for your platform! (otherwise select() won't work)"
#endif





// fwd:
class cSocket;
class cClientHandle;





class cSocketThreads
{
public:

	// Clients of cSocketThreads must implement this interface to be able to communicate
	class cCallback
	{
	public:
		/// Called when data is received from the remote party
		virtual void DataReceived(const char * a_Data, int a_Size) = 0;
		
		/// Called when data can be sent to remote party; the function is supposed to append outgoing data to a_Data
		virtual void GetOutgoingData(AString & a_Data) = 0;
		
		/// Called when the socket has been closed for any reason
		virtual void SocketClosed(void) = 0;
	} ;

	
	cSocketThreads(void);
	~cSocketThreads();
	
	/// Add a (socket, client) pair for processing, data from a_Socket is to be sent to a_Client; returns true if successful
	bool AddClient(cSocket * a_Socket, cCallback * a_Client);
	
	/// Remove the socket (and associated client) from processing
	void RemoveClient(const cSocket * a_Socket);
	
	/// Remove the associated socket and the client from processing
	void RemoveClient(const cCallback * a_Client);
	
	/// Notify the thread responsible for a_Client that the client has something to write
	void NotifyWrite(const cCallback * a_Client);

private:

	class cSocketThread :
		public cIsThread
	{
		typedef cIsThread super;
		
	public:
	
		cSocketThread(cSocketThreads * a_Parent);
		
		// All these methods assume parent's m_CS is locked
		bool HasEmptySlot(void) const {return m_NumSlots < MAX_SLOTS; }
		bool IsEmpty     (void) const {return m_NumSlots == 0; }

		void AddClient   (cSocket *         a_Socket, cCallback * a_Client);
		bool RemoveClient(const cCallback * a_Client);  // Returns true if removed, false if not found
		bool RemoveSocket(const cSocket *   a_Socket);  // Returns true if removed, false if not found
		bool HasClient   (const cCallback * a_Client) const;
		bool HasSocket   (const cSocket *   a_Socket) const;
		bool NotifyWrite (const cCallback * a_Client);  // Returns true if client handled by this thread
		
		bool Start(void);  // Hide the cIsThread's Start method, we need to provide our own startup to create the control socket
		
		bool IsValid(void) const {return m_ControlSocket2.IsValid(); }  // If the Control socket dies, the thread is not valid anymore
		
	private:
	
		cSocketThreads * m_Parent;
	
		// Two ends of the control socket, the first is select()-ed, the second is written to for notifications
		cSocket m_ControlSocket1;
		cSocket m_ControlSocket2;
		
		// Socket-client-packetqueues triplets.
		// Manipulation with these assumes that the parent's m_CS is locked
		struct sSlot
		{
			cSocket *   m_Socket;
			cCallback * m_Client;
			AString     m_Outgoing;  // If sending writes only partial data, the rest is stored here for another send
		} ;
		sSlot m_Slots[MAX_SLOTS];
		int   m_NumSlots;  // Number of slots actually used
		
		virtual void Execute(void) override;
		
		void AddOrUpdatePacket(int a_Slot, cPacket * a_Packet);  // Adds the packet to the specified slot, or updates an existing packet in that queue (EntityMoveLook filtering)
		
		void PrepareSet     (fd_set * a_Set, cSocket::xSocket & a_Highest);  // Puts all sockets into the set, along with m_ControlSocket1
		void ReadFromSockets(fd_set * a_Read);  // Reads from sockets indicated in a_Read
		void WriteToSockets (fd_set * a_Write);  // Writes to sockets indicated in a_Write
		void RemoveClosedSockets(void);  // Removes sockets that have closed from m_Slots[]
	} ;
	
	typedef std::list<cSocketThread *> cSocketThreadList;
	
	
	cCriticalSection  m_CS;
	cSocketThreadList m_Threads;
} ;





#endif  // CSOCKETTHREADS_H_INCLUDED



