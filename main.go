package main

import (
	"able/lexer"
	"bytes"
	"fmt"
	"io/ioutil"
	"log"
	"os"
)

func main() {
	if len(os.Args) < 2 {
		fmt.Println("Able needs a source file. Execute a program by typing: able <file name>")
		os.Exit(1)
	}

	fileName := os.Args[1]
	fmt.Printf("Running file: %s\n", fileName)

	file, err := os.Open(fileName)

	if err != nil {
		log.Fatal(err)
	}

	b, err := ioutil.ReadAll(file)
	if err != nil {
		log.Fatal(err)
	}
	var source bytes.Buffer
	source.Write(b)

	l := lexer.New(source.String())
	l.GenerateProto()
}
