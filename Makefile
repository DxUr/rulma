all:
	.bake/bake build

run-test:
	.bake/bake run -a ./test/test.rl
