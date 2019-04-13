trait Ord {
  fn lt(a: Self, b: Self) -> Bool
  fn le(a: Self, b: Self) -> Bool
  fn gt(a: Self, b: Self) -> Bool
  fn ge(a: Self, b: Self) -> Bool
}

trait Eq {
  fn eq(a: Self, b: Self) -> Bool
}

struct MyAggregate {
  contents: Array<T> where T: Ord, Eq
}

impl MyAggregate {
  fn get_value(self, at: Number) {
    return self.contents[at]
  }
}

struct MyGeneric<T> {
  contents: T
}

struct MyString {
  contents: Array<Char>
}

impl Eq for MyString {
  fn eq(a: MyString, b: MyString) {
    if a.contents.length != b.contents.length {
      return false
    }

    for i in 0..a.contents.length {
      if a.contents[i] != b.contents[i] {
        return false
      }
    }

    return true
  }
}

//  Issue:
//    What if Number class defines fn _eq_(a: Number, b: MyString) ?
//    Ambiguity: let b = MyString::_eq_(a, b)

fn main() {
  let str1 = MyString {
    contents: ["a", "b", "c"]
  }

  let str2 = MyString {
    contents: ["b", "c", "d"]
  }

  //  error: Type "MyString" is not assignable to Array<T> where T: Ord & Eq
  //    Type does not implement "Ord"
  let aggregate1 = MyAggregate {
    contents: [1, 2, "hi", str1]
  }

  println(str1 == str2)
}