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

	hasher := hmac.New(sha1.New, key)
	fmt.Printf("Key length: %v\n", len(key))
	fmt.Printf("Recommended key length: %v\n", hasher.BlockSize())
	var i = 0
	for {
		time.Sleep(time.Millisecond*50)
		conn.SetDeadline(time.Now().Add(time.Second * 5))

		var result []byte = make([]byte, 12)
		fmt.Printf("Sending request\n")
		_, err = conn.Write([]byte{0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})
		if err != nil {
			log.Fatal(err)
		}

		r_len, err := conn.Read(result)
		if err != nil {
			log.Printf("Read failed, retrying: i=%v, %v\n", i, err)
			i++
			continue
		} else {
			log.Printf("Got 0x%x\n", result[:r_len])
		}

		_, err = hasher.Write(result[:r_len])
		if err != nil {
			log.Fatal(err)
		}
		packet := hasher.Sum([]byte{0x01, 0x01})
		hasher.Reset()

		fmt.Printf("Sending solution: %x\n", packet)
		_, err = conn.Write(packet)
		log.Println("Sent bytes")
		if err != nil {
			log.Fatal(err)
		}
		_, err = conn.Read(result)
		if err != nil {
			log.Printf("Read failed, retrying: i=%v, %v\n", i, err)
			i++
		} else {
			log.Println("Got ", result)
			break
		}
	}
}
