package main

import (
	"fmt"
	"cmp"
)

func MapValues[T cmp.Ordered](values []T, mapFunc func(T) T) []T {
	var newValues []T
	for _, v := range values {
		newValue := mapFunc(v)
		newValues = append(newValues, newValue)
	}
	return newValues
}

func main() {

	result := MapValues([]int{1, 2, 3}, func(n int) int {
		return n*2
	})

	fmt.Printf("result: %+v\n", result)
}