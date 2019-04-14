struct Test {
  member1: Int
  member2: Int
}

trait Ord {
  fn lt(Self, other: Self) -> Bool;
  fn gt(Self, other: Self) -> Bool;
}


fn another<T>(a: T) -> T where T: {Ord, Eq} {
  return a;
}

fn main() -> void {
  let x = another(1) + 2
    - 2 * another(2);

  x = 10;
}