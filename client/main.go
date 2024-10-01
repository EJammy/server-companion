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
		log.Fatal("Too much argument!")
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

	var result []byte = make([]byte, 12)
	var byte_read = 0
	for byte_read <= 0 {
        h := hmac.New(sha1.New, key)
		_, err = h.Write([]byte("request"))
		if err != nil {
			log.Fatal(err)
		}
		fmt.Printf("Key length: %v\n", len(key))
		fmt.Printf("Recommended key length: %v\n", h.BlockSize())
		packet := h.Sum([]byte{0x01, 0x01})
		fmt.Printf("Sending packet: %x\n", packet)
		_, err = conn.Write(packet)
		log.Println("Sent bytes")
		if err != nil {
			log.Fatal(err)
		}
		// TODO: add timeout
		byte_read, err = conn.Read(result)
		if err != nil {
			log.Println("Read failed, retrying: ", err)
		}
		log.Println("Got ", result)
		time.Sleep(time.Second)
	}
}
