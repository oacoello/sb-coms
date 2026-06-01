package main

import (
	"log"
	"net"
	"time"
)

const listenAddress = ":50000"

type client struct {
	addr     *net.UDPAddr
	lastSeen time.Time
}

func main() {
	addr, err := net.ResolveUDPAddr("udp", listenAddress)
	if err != nil {
		log.Fatal(err)
	}

	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		log.Fatal(err)
	}
	defer conn.Close()

	log.Printf("sb-coms relay listening on udp %s", listenAddress)

	clients := map[string]client{}
	buffer := make([]byte, 4096)

	for {
		n, remote, err := conn.ReadFromUDP(buffer)
		if err != nil {
			log.Printf("read error: %v", err)
			continue
		}

		key := remote.String()
		clients[key] = client{addr: remote, lastSeen: time.Now()}

		for otherKey, other := range clients {
			if otherKey == key {
				continue
			}

			if time.Since(other.lastSeen) > 30*time.Second {
				delete(clients, otherKey)
				continue
			}

			if _, err := conn.WriteToUDP(buffer[:n], other.addr); err != nil {
				log.Printf("write to %s failed: %v", other.addr, err)
			}
		}
	}
}
