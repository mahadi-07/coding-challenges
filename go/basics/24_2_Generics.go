package main

import (
	"fmt"
	"cmp"
)

type CustomData interface {
	cmp.Ordered | []byte | []rune
}

type User[T CustomData] struct {
	ID		int
	Name	string
	Data	T
}

func main() {
	u := User[int] {
		ID: 	0,
		Name:	"shakil",
		Data:	3,
	}
	fmt.Println("user: %+v\n", u)

	u1 := User[string] {
		ID: 	0,
		Name:	"shakil",
		Data:	"fkjsflkjslkfjsf",
	}
	fmt.Println("user1: %+v\n", u1)
}