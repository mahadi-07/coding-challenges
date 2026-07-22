package main

import (
	"fmt"
	"slices"
)

func assert(cond bool, msg string) {
	if !cond {
		panic(msg)
	}
}

func Filter(s []int, fn func(int) bool) []int {
	var p []int

	for _, v := range s {
		if fn(v) {
			p = append(p, v)
		}
	}
	return p
}

func main() {
	letters := []string{"a", "b", "c", "d"}
	fmt.Println(letters)

	var s []int
	s = make([]int, 5, 5)
	s = append(s, 1, 2, 3, 4, 5, 6)
	fmt.Println(len(s), cap(s))

	// The important point is that t does not contain a copy of the data.
	// It is a slice that shares the same underlying array as b
	// Underlying array
	// Index: 0   1   2   3   4   5
	// 	g   o   l   a   n   g
	// 	↑   ↑   ↑   ↑
	// 	│   └──────────────┐
	// 	└──────────────────┘
	// 			t = b[1:4]
	// 			len=3 cap=5
	b := []byte{'g', 'o', 'l', 'a', 'n', 'g'}
	t := b[1:4]
	fmt.Println(t)

	// Since b and t share the same storage, changing one changes the other.
	t[0] = 'x'
	fmt.Println(b)

	
	// If we don't want to share the underlying array, make a copy
	// two ways to do that
	copySlice1 := append([]byte(nil), b[1:4]...)
	fmt.Println(copySlice1)

	copySlice2 := make([]byte, 3)
	copy(copySlice2, b[1:4])
	fmt.Println(copySlice2)

	// 
	x := [3]string{"Лайка", "Белка", "Стрелка"}
	sx := x[:]
	sx[0] = "fklasjdlfkasdf"
	fmt.Println(x)
	fmt.Println(sx)
	assert(slices.Equal(x[:], sx), "lfkjskfljsdf")

	sa := []int{1, 2, 3, 4, 5}
	ta := make([]int, 3)
	copy(ta, sa[2:])
	fmt.Println(t, len(ta), cap(ta))

	for i, v := range sa[2:] {
		fmt.Println(i, v)
	}
	// fmt.Println(tmp...)

	// 

	nums := []int{1, 2, 3, 4, 5, 6}

	cmpFunc := func(x int) bool {
		if x % 2 == 0 {
			return true
		}
		return false
	}

	even := Filter(nums, cmpFunc);
	fmt.Println("even ", even)
}