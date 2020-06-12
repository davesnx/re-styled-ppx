type error = string;
type data('a) = result('a, error);
type rule('a) = list(Tokens.token) => (data('a), list(Tokens.token));

type return('a, 'b) = 'b => rule('a);
type bind('a, 'b, 'c) = (rule('a), 'b => rule('c)) => rule('c);
type best('left_in, 'left_v, 'right_in, 'right_v, 'c) =
  (
    (rule('left_in), rule('right_in)),
    [ | `Left('left_v) | `Right('right_v)] => rule('c)
  ) =>
  rule('c);

// monad to deal with tokens
module Data = {
  let return = (data, tokens) => (data, tokens);
  let bind = (rule, f, tokens) => {
    let (data, tokens) = rule(tokens);
    f(data, tokens);
  };
  let bind_shortest_or_longest = (shortest, (left, right), f, tokens) => {
    let (left_data, left_tokens) = left(tokens);
    let (right_data, right_tokens) = right(tokens);
    let op = shortest ? (>) : (<);
    let use_left = op(List.length(left_tokens), List.length(right_tokens));
    use_left
      ? f(`Left(left_data), left_tokens)
      : f(`Right(right_data), right_tokens);
  };
  let bind_shortest: best('a, data('a), 'b, data('b), 'c) =
    ((left, right), f) => bind_shortest_or_longest(true, (left, right), f);
  let bind_longest: best('a, data('a), 'b, data('b), 'c) =
    ((left, right), f) =>
      bind_shortest_or_longest(false, (left, right), f);
};

// monad when match is successful
module Match = {
  let return = value => Data.return(Ok(value));
  let bind: bind('a, 'a, 'b) =
    (rule, f) =>
      Data.bind(
        rule,
        fun
        | Ok(value) => f(value)
        | Error(error) => Data.return(Error(error)),
      );
  let bind_shortest_or_longest = (shortest, (left, right), f, tokens) => {
    let (left_data, left_tokens) = left(tokens);
    let (right_data, right_tokens) = right(tokens);
    let op = shortest ? (>) : (<);
    let use_left = op(List.length(left_tokens), List.length(right_tokens));
    switch (left_data, right_data) {
    | (Ok(left_value), Error(_)) => f(`Left(left_value), left_tokens)
    | (Error(_), Ok(right_value)) => f(`Right(right_value), right_tokens)
    | (Ok(left_value), Ok(right_value)) =>
      use_left
        ? f(`Left(left_value), left_tokens)
        : f(`Right(right_value), right_tokens)
    | (Error(left_data), Error(right_data)) =>
      use_left
        ? Data.return(Error(left_data), left_tokens)
        : Data.return(Error(right_data), right_tokens)
    };
  };
  let bind_shortest = ((left, right), f) =>
    bind_shortest_or_longest(true, (left, right), f);
  let bind_longest = ((left, right), f) =>
    bind_shortest_or_longest(false, (left, right), f);
};

module Let = {
  let return_data = Data.return;
  let (let.bind_data) = Data.bind;
  let (let.bind_shortest_data) = Data.bind_shortest;
  let (let.bind_longest_data) = Data.bind_longest;

  let return_match = Match.return;
  let (let.bind_match) = Match.bind;
  let (let.bind_shortest_match) = Match.bind_shortest;
  let (let.bind_longest_match) = Match.bind_longest;
};

module Pattern = {
  // TODO: errors
  let identity = Match.return();
  let token = expected =>
    fun
    | [token, ...tokens] => (expected(token), tokens)
    | [] => (Error("missing the token expected"), []);
  let expect = expected =>
    token(
      fun
      | token when token == expected => Ok()
      | _ => Error("expected "),
    );
  let value = (value, rule) => Match.bind(rule, () => Match.return(value));
};
