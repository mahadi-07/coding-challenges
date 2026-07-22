package main

import (
	"fmt"
	"regexp"
	"io/ioutil"
)

var digitRegexp = regexp.MustCompile("[0-9]+")

func FindDigits(filename string) []byte {
	b, err := ioutil.ReadFile(filename)
	if err != nil {
		panic(err)
	}

	res := digitRegexp.Find(b)
	fmt.Println(b, len(b), cap(b))

	b[8] = 'X'

	fmt.Printf("&b[0]   = %p\n", &b[6])
	fmt.Printf("&res[0] = %p\n", &res[0])


	fmt.Printf("&b[0]   = %p\n", &b[7])
	fmt.Printf("&res[0] = %p\n", &res[1])

	return res
}

func main() {

	res := FindDigits("./data/9_2.txt")
	fmt.Printf("&res inside main[0] = %p\n", &res[0])
	fmt.Println("res ", string(res), len(res), cap(res))
}