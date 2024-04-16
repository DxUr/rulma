

let E =

let T = space {

}


let false : static int = 1+(1*(5))
let true = 1

let alloc = space {
	fn new(T: @type) &T {
		ret alloc(T.size)
		let c = @assigned.count
		if !c {
			dealloc(@ret)
		}
	}
}

let Square = type{lenght int}

fn get_area(self: &Square) {
	ret self.lenght * self.lenght
}

let anes : const = "Good Job"


let T = type {
	a: int
}


fn typeof(T: @type) string {
	ret string(T.name)
}


fn Optional(T: @type) @type {
	ret enum {
		Some(T),
		None
	}
}


fn get_val() Optional!(int) {
	ret 5
}

fn Vector(T: @type, n: int) @type {
	ret T[n]
}

fn foo() {
	let a = alloc.new(int)
	let b = a

	let a: Optional(int)
	match a {
		Some(v) {}
		None {}
	}

}
