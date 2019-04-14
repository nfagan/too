struct Test {
  member1: Number
}

fn another<T>(a: T) -> T where T: {Ord, Eq} {
  return a
}

fn main() -> void {
  let x = another(1)
}