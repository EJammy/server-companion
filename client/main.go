package main

import (
	"crypto/hmac"
	"crypto/sha1"
	"fmt"
	"log"
	"net"
	"time"
	"os"
)

func main() {
	fmt.Println("Hello world")

	if len(os.Args) > 3 {
		log.Fatal("Too many argument!")
	}
	if len(os.Args) < 2 {
		log.Fatal("No IP address provided!")
	}
	var companionAddr = os.Args[1]

	key_filename := "secret_key.bin"
	if len(os.Args) > 2 {
		key_filename = os.Args[2]
	}

	key, err := os.ReadFile(key_filename)
	if err != nil {
		log.Fatal(err)
	}

	// Use Dial because we only send to and recieve from one address
	conn, err := net.Dial("udp", companionAddr)
	if err != nil {
		log.Fatal(err)
	}

	h := hmac.New(sha1.New, key)
	fmt.Printf("Key length: %v\n", len(key))
	fmt.Printf("Recommended key length: %v\n", h.BlockSize())
	packet := h.Sum([]byte{0x01, 0x01})
	fmt.Printf("Sending packet: %x\n", packet)
	var i = 0
	for {
		_, err = h.Write([]byte("request"))
		if err != nil {
			log.Fatal(err)
		}
		conn.SetDeadline(time.Now().Add(time.Second * 2))
		_, err = conn.Write(packet)
		log.Println("Sent bytes")
		if err != nil {
			log.Fatal(err)
		}
		var result []byte = make([]byte, 12)
		_, err = conn.Read(result)
		if err != nil {
			log.Printf("Read failed, retrying: i=%v, %v\n", i, err)
			i++
		} else {
			log.Println("Got ", result)
			break
		}
		time.Sleep(time.Millisecond*50)
	}
}
