open Migrate_parsetree;
open Ast_410;
open Ast_helper;
open Longident;
open Reason_css_parser;
open Reason_css_lexer;
open Parser;

/* helpers */
let txt = txt => {Location.loc: Location.none, txt};
let lid = name => txt(Lident(name));

let (let.ok) = Result.bind;

exception Unsupported_feature;

let id = Fun.id;
let apply_value = (f, v) => f(`Value(v));

type transform('ast, 'value) = {
  ast_of_string: string => result('ast, string),
  value_of_ast: 'ast => 'value,
  value_to_expr: 'value => list(Parsetree.expression),
  ast_to_expr: 'ast => list(Parsetree.expression),
  string_to_expr: string => result(list(Parsetree.expression), string),
};

let transform = (parser, value_of_ast, value_to_expr) => {
  let ast_of_string = Parser.parse(parser);
  let ast_to_expr = ast => value_of_ast(ast) |> value_to_expr;
  let string_to_expr = string =>
    ast_of_string(string) |> Result.map(ast_to_expr);
  {ast_of_string, value_of_ast, value_to_expr, ast_to_expr, string_to_expr};
};
let unsupported = _parser =>
  transform(
    property_block_ellipsis,
    fun
    | _ => raise(Unsupported_feature),
    fun
    | _ => raise(Unsupported_feature),
  );
let render_css_wide_keywords = (name, value) => {
  let.ok value = Parser.parse(Standard.css_wide_keywords, value);
  let value =
    switch (value) {
    | `Inherit =>
      %expr
      "inherit"
    | `Initial =>
      %expr
      "initial"
    | `Unset =>
      %expr
      "unset"
    };
  let name = Const.string(name) |> Exp.constant;
  Ok([[%expr Css.unsafe([%e name], [%e value])]]);
};

let render_string = string => Const.string(string) |> Exp.constant;
let render_integer = integer => Const.int(integer) |> Exp.constant;
let render_number = number =>
  Const.float(number |> string_of_float) |> Exp.constant;
let render_percentage = number => [%expr
  `percent([%e render_number(number)])
];
let render_angle =
  fun
  | `Deg(number) => id([%expr `deg([%e render_number(number)])])
  | `Rad(number) => id([%expr `rad([%e render_number(number)])])
  | `Grad(number) => id([%expr `grad([%e render_number(number)])])
  | `Turn(number) => id([%expr `turn([%e render_number(number)])]);

let variants_to_expression =
  fun
  | `Row => id([%expr `row])
  | `Row_reverse => id([%expr `rowReverse])
  | `Column => id([%expr `column])
  | `Column_reverse => id([%expr `columnReverse])
  | `Nowrap => id([%expr `nowrap])
  | `Wrap => id([%expr `wrap])
  | `Wrap_reverse => id([%expr `wrapReverse])
  | `Content => id([%expr `content])
  | `Flex_start => id([%expr `flexStart])
  | `Flex_end => id([%expr `flexEnd])
  | `Center => id([%expr `center])
  | `Space_between => id([%expr `spaceBetween])
  | `Space_around => id([%expr `spaceAround])
  | `Baseline => id([%expr `baseline])
  | `Stretch => id([%expr `stretch])
  | `Auto => id([%expr `auto])
  | `None => id([%expr `none])
  | `Content_box => id([%expr `contentBox])
  | `Border_box => id([%expr `borderBox])
  | `Clip => id([%expr `clip])
  | `Hidden => id([%expr `hidden])
  | `Visible => id([%expr `visible])
  | `Scroll => id([%expr `scroll])
  | `Ellipsis => id([%expr `ellipsis])
  | `Capitalize => id([%expr `capitalize])
  | `Lowercase => id([%expr `lowercase])
  | `Uppercase => id([%expr `uppercase])
  | `Break_spaces => id([%expr `breakSpaces])
  | `Normal => id([%expr `normal])
  | `Pre => id([%expr `pre])
  | `Pre_line => id([%expr `preLine])
  | `Pre_wrap => id([%expr `preWrap])
  | `Break_all => id([%expr `breakAll])
  | `Break_word => raise(Unsupported_feature)
  | `Keep_all => id([%expr `keepAll])
  | `Anywhere => id([%expr `anywhere])
  | `End => raise(Unsupported_feature)
  | `Justify => id([%expr `justify])
  | `Justify_all => raise(Unsupported_feature)
  | `Left => id([%expr `left])
  | `Match_parent => raise(Unsupported_feature)
  | `Right => id([%expr `right])
  | `Start => id([%expr `start])
  | `Currentcolor => id([%expr `currentColor])
  | `Transparent => id([%expr `transparent])
  | `Bottom => id([%expr `bottom])
  | `Top => id([%expr `top])
  | `Fill => id([%expr `fill]);

let variable_rule = {
  open Rule;
  open Let;

  let.bind_match () = Pattern.expect(DELIM("$"));
  let.bind_match _ = Pattern.expect(LEFT_PARENS) |> Modifier.optional;
  let.bind_match string = Standard.ident;
  let.bind_match _ = Pattern.expect(RIGHT_PARENS) |> Modifier.optional;
  return_match(string);
};
let variable = parser =>
  Combinator.combine_xor([
    Rule.Match.map(variable_rule, data => `Variable(data)),
    Rule.Match.map(parser, data => `Value(data)),
  ]);
let apply = (parser, map, id) =>
  transform(
    variable(parser),
    fun
    | `Variable(name) => name |> lid |> Exp.ident
    | `Value(ast) => map(ast),
    arg =>
    [[%expr [%e id]([%e arg])]]
  );

let variants = (parser, identifier) =>
  apply(parser, variants_to_expression, identifier);

// TODO: all of them could be float, but bs-css doesn't support it
let render_length =
  fun
  | `Cap(_n) => raise(Unsupported_feature)
  | `Ch(n) => [%expr `ch([%e render_number(n)])]
  | `Cm(n) => [%expr `cm([%e render_number(n)])]
  | `Em(n) => [%expr `em([%e render_number(n)])]
  | `Ex(n) => [%expr `ex([%e render_number(n)])]
  | `Ic(_n) => raise(Unsupported_feature)
  | `In(_n) => raise(Unsupported_feature)
  | `Lh(_n) => raise(Unsupported_feature)
  | `Mm(n) => [%expr `mm([%e render_number(n)])]
  | `Pc(n) => [%expr `pc([%e render_number(n)])]
  | `Pt(n) => [%expr `pt([%e render_integer(n |> int_of_float)])]
  | `Px(n) => [%expr `pxFloat([%e render_number(n)])]
  | `Q(_n) => raise(Unsupported_feature)
  | `Rem(n) => [%expr `rem([%e render_number(n)])]
  | `Rlh(_n) => raise(Unsupported_feature)
  | `Vb(_n) => raise(Unsupported_feature)
  | `Vh(n) => [%expr `vh([%e render_number(n)])]
  | `Vi(_n) => raise(Unsupported_feature)
  | `Vmax(n) => [%expr `vmax([%e render_number(n)])]
  | `Vmin(n) => [%expr `vmin([%e render_number(n)])]
  | `Vw(n) => [%expr `vw([%e render_number(n)])]
  | `Zero => [%expr `zero];

let render_length_percentage =
  fun
  | `Length(length) => render_length(length)
  | `Percentage(percentage) => render_percentage(percentage);

// css-sizing-3
let render_function_fit_content = _lp => raise(Unsupported_feature);
let width =
  apply(
    property_width,
    fun
    | `Auto => variants_to_expression(`Auto)
    | `Length_percentage(lp) => render_length_percentage(lp)
    | `Max_content
    | `Min_content => raise(Unsupported_feature)
    | `Fit_content(lp) => render_function_fit_content(lp),
    [%expr Css.width],
  );
let height =
  apply(
    property_height,
    apply_value(width.value_of_ast),
    [%expr Css.height],
  );
let min_width =
  apply(
    property_min_width,
    apply_value(width.value_of_ast),
    [%expr Css.minWidth],
  );
let min_height =
  apply(
    property_min_height,
    apply_value(width.value_of_ast),
    [%expr Css.minHeight],
  );
let max_width =
  apply(
    property_max_width,
    fun
    | `None => variants_to_expression(`None)
    | `Length_percentage(lp) =>
      apply_value(width.value_of_ast, `Length_percentage(lp))
    | `Max_content => apply_value(width.value_of_ast, `Max_content)
    | `Min_content => apply_value(width.value_of_ast, `Min_content)
    | `Fit_content(lp) => apply_value(width.value_of_ast, `Fit_content(lp)),
    [%expr Css.maxWidth],
  );
let max_height =
  apply(
    property_max_height,
    data => max_width.value_of_ast(`Value(data)),
    [%expr Css.maxHeight],
  );
let box_sizing =
  apply(property_box_sizing, variants_to_expression, [%expr Css.boxSizing]);
let column_width = unsupported(property_column_width);

// css-box-3
let margin_top =
  apply(
    property_margin_top,
    fun
    | `Auto => variants_to_expression(`Auto)
    | `Length_percentage(lp) => render_length_percentage(lp),
    [%expr Css.marginTop],
  );
let margin_right =
  apply(
    property_margin_right,
    apply_value(margin_top.value_of_ast),
    [%expr Css.marginRight],
  );
let margin_bottom =
  apply(
    property_margin_bottom,
    apply_value(margin_top.value_of_ast),
    [%expr Css.marginBottom],
  );
let margin_left =
  apply(
    property_margin_left,
    apply_value(margin_top.value_of_ast),
    [%expr Css.marginLeft],
  );
let margin =
  transform(
    property_margin,
    List.map(apply_value(margin_top.value_of_ast)),
    fun
    | [all] => [[%expr Css.margin([%e all])]]
    | [v, h] => [[%expr Css.margin2(~v=[%e v], ~h=[%e h])]]
    | [t, h, b] => [
        [%expr Css.margin3(~top=[%e t], ~h=[%e h], ~bottom=[%e b])],
      ]
    | [t, r, b, l] => [
        [%expr
          Css.margin4(
            ~top=[%e t],
            ~right=[%e r],
            ~bottom=[%e b],
            ~left=[%e l],
          )
        ],
      ]
    | _ => failwith("unreachable"),
  );
let padding_top =
  apply(
    property_padding_top,
    render_length_percentage,
    [%expr Css.paddingTop],
  );
let padding_right =
  apply(
    property_padding_right,
    apply_value(padding_top.value_of_ast),
    [%expr Css.paddingRight],
  );
let padding_bottom =
  apply(
    property_padding_bottom,
    apply_value(padding_top.value_of_ast),
    [%expr Css.paddingBottom],
  );
let padding_left =
  apply(
    property_padding_left,
    apply_value(padding_top.value_of_ast),
    [%expr Css.paddingLeft],
  );
let padding =
  transform(
    property_padding,
    List.map(apply_value(padding_top.value_of_ast)),
    fun
    | [all] => [[%expr Css.padding([%e all])]]
    | [v, h] => [[%expr Css.padding2(~v=[%e v], ~h=[%e h])]]
    | [t, h, b] => [
        [%expr Css.padding3(~top=[%e t], ~h=[%e h], ~bottom=[%e b])],
      ]
    | [t, r, b, l] => [
        [%expr
          Css.padding4(
            ~top=[%e t],
            ~right=[%e r],
            ~bottom=[%e b],
            ~left=[%e l],
          )
        ],
      ]
    | _ => failwith("unreachable"),
  );

let render_named_color =
  fun
  | `Aliceblue => [%expr Css.aliceblue]
  | `Antiquewhite => [%expr Css.antiquewhite]
  | `Aqua => [%expr Css.aqua]
  | `Aquamarine => [%expr Css.aquamarine]
  | `Azure => [%expr Css.azure]
  | `Beige => [%expr Css.beige]
  | `Bisque => [%expr Css.bisque]
  | `Black => [%expr Css.black]
  | `Blanchedalmond => [%expr Css.blanchedalmond]
  | `Blue => [%expr Css.blue]
  | `Blueviolet => [%expr Css.blueviolet]
  | `Brown => [%expr Css.brown]
  | `Burlywood => [%expr Css.burlywood]
  | `Cadetblue => [%expr Css.cadetblue]
  | `Chartreuse => [%expr Css.chartreuse]
  | `Chocolate => [%expr Css.chocolate]
  | `Coral => [%expr Css.coral]
  | `Cornflowerblue => [%expr Css.cornflowerblue]
  | `Cornsilk => [%expr Css.cornsilk]
  | `Crimson => [%expr Css.crimson]
  | `Cyan => [%expr Css.cyan]
  | `Darkblue => [%expr Css.darkblue]
  | `Darkcyan => [%expr Css.darkcyan]
  | `Darkgoldenrod => [%expr Css.darkgoldenrod]
  | `Darkgray => [%expr Css.darkgray]
  | `Darkgreen => [%expr Css.darkgreen]
  | `Darkgrey => [%expr Css.darkgrey]
  | `Darkkhaki => [%expr Css.darkkhaki]
  | `Darkmagenta => [%expr Css.darkmagenta]
  | `Darkolivegreen => [%expr Css.darkolivegreen]
  | `Darkorange => [%expr Css.darkorange]
  | `Darkorchid => [%expr Css.darkorchid]
  | `Darkred => [%expr Css.darkred]
  | `Darksalmon => [%expr Css.darksalmon]
  | `Darkseagreen => [%expr Css.darkseagreen]
  | `Darkslateblue => [%expr Css.darkslateblue]
  | `Darkslategray => [%expr Css.darkslategray]
  | `Darkslategrey => [%expr Css.darkslategrey]
  | `Darkturquoise => [%expr Css.darkturquoise]
  | `Darkviolet => [%expr Css.darkviolet]
  | `Deeppink => [%expr Css.deeppink]
  | `Deepskyblue => [%expr Css.deepskyblue]
  | `Dimgray => [%expr Css.dimgray]
  | `Dimgrey => [%expr Css.dimgrey]
  | `Dodgerblue => [%expr Css.dodgerblue]
  | `Firebrick => [%expr Css.firebrick]
  | `Floralwhite => [%expr Css.floralwhite]
  | `Forestgreen => [%expr Css.forestgreen]
  | `Fuchsia => [%expr Css.fuchsia]
  | `Gainsboro => [%expr Css.gainsboro]
  | `Ghostwhite => [%expr Css.ghostwhite]
  | `Gold => [%expr Css.gold]
  | `Goldenrod => [%expr Css.goldenrod]
  | `Gray => [%expr Css.gray]
  | `Green => [%expr Css.green]
  | `Greenyellow => [%expr Css.greenyellow]
  | `Grey => [%expr Css.grey]
  | `Honeydew => [%expr Css.honeydew]
  | `Hotpink => [%expr Css.hotpink]
  | `Indianred => [%expr Css.indianred]
  | `Indigo => [%expr Css.indigo]
  | `Ivory => [%expr Css.ivory]
  | `Khaki => [%expr Css.khaki]
  | `Lavender => [%expr Css.lavender]
  | `Lavenderblush => [%expr Css.lavenderblush]
  | `Lawngreen => [%expr Css.lawngreen]
  | `Lemonchiffon => [%expr Css.lemonchiffon]
  | `Lightblue => [%expr Css.lightblue]
  | `Lightcoral => [%expr Css.lightcoral]
  | `Lightcyan => [%expr Css.lightcyan]
  | `Lightgoldenrodyellow => [%expr Css.lightgoldenrodyellow]
  | `Lightgray => [%expr Css.lightgray]
  | `Lightgreen => [%expr Css.lightgreen]
  | `Lightgrey => [%expr Css.lightgrey]
  | `Lightpink => [%expr Css.lightpink]
  | `Lightsalmon => [%expr Css.lightsalmon]
  | `Lightseagreen => [%expr Css.lightseagreen]
  | `Lightskyblue => [%expr Css.lightskyblue]
  | `Lightslategray => [%expr Css.lightslategray]
  | `Lightslategrey => [%expr Css.lightslategrey]
  | `Lightsteelblue => [%expr Css.lightsteelblue]
  | `Lightyellow => [%expr Css.lightyellow]
  | `Lime => [%expr Css.lime]
  | `Limegreen => [%expr Css.limegreen]
  | `Linen => [%expr Css.linen]
  | `Magenta => [%expr Css.magenta]
  | `Maroon => [%expr Css.maroon]
  | `Mediumaquamarine => [%expr Css.mediumaquamarine]
  | `Mediumblue => [%expr Css.mediumblue]
  | `Mediumorchid => [%expr Css.mediumorchid]
  | `Mediumpurple => [%expr Css.mediumpurple]
  | `Mediumseagreen => [%expr Css.mediumseagreen]
  | `Mediumslateblue => [%expr Css.mediumslateblue]
  | `Mediumspringgreen => [%expr Css.mediumspringgreen]
  | `Mediumturquoise => [%expr Css.mediumturquoise]
  | `Mediumvioletred => [%expr Css.mediumvioletred]
  | `Midnightblue => [%expr Css.midnightblue]
  | `Mintcream => [%expr Css.mintcream]
  | `Mistyrose => [%expr Css.mistyrose]
  | `Moccasin => [%expr Css.moccasin]
  | `Navajowhite => [%expr Css.navajowhite]
  | `Navy => [%expr Css.navy]
  | `Oldlace => [%expr Css.oldlace]
  | `Olive => [%expr Css.olive]
  | `Olivedrab => [%expr Css.olivedrab]
  | `Orange => [%expr Css.orange]
  | `Orangered => [%expr Css.orangered]
  | `Orchid => [%expr Css.orchid]
  | `Palegoldenrod => [%expr Css.palegoldenrod]
  | `Palegreen => [%expr Css.palegreen]
  | `Paleturquoise => [%expr Css.paleturquoise]
  | `Palevioletred => [%expr Css.palevioletred]
  | `Papayawhip => [%expr Css.papayawhip]
  | `Peachpuff => [%expr Css.peachpuff]
  | `Peru => [%expr Css.peru]
  | `Pink => [%expr Css.pink]
  | `Plum => [%expr Css.plum]
  | `Powderblue => [%expr Css.powderblue]
  | `Purple => [%expr Css.purple]
  | `Rebeccapurple => [%expr Css.rebeccapurple]
  | `Red => [%expr Css.red]
  | `Rosybrown => [%expr Css.rosybrown]
  | `Royalblue => [%expr Css.royalblue]
  | `Saddlebrown => [%expr Css.saddlebrown]
  | `Salmon => [%expr Css.salmon]
  | `Sandybrown => [%expr Css.sandybrown]
  | `Seagreen => [%expr Css.seagreen]
  | `Seashell => [%expr Css.seashell]
  | `Sienna => [%expr Css.sienna]
  | `Silver => [%expr Css.silver]
  | `Skyblue => [%expr Css.skyblue]
  | `Slateblue => [%expr Css.slateblue]
  | `Slategray => [%expr Css.slategray]
  | `Slategrey => [%expr Css.slategrey]
  | `Snow => [%expr Css.snow]
  | `Springgreen => [%expr Css.springgreen]
  | `Steelblue => [%expr Css.steelblue]
  | `Tan => [%expr Css.tan]
  | `Teal => [%expr Css.teal]
  | `Thistle => [%expr Css.thistle]
  | `Tomato => [%expr Css.tomato]
  | `Turquoise => [%expr Css.turquoise]
  | `Violet => [%expr Css.violet]
  | `Wheat => [%expr Css.wheat]
  | `White => [%expr Css.white]
  | `Whitesmoke => [%expr Css.whitesmoke]
  | `Yellow => [%expr Css.yellow]
  | `Yellowgreen => [%expr Css.yellowgreen];
let render_color_alpha =
  fun
  | `Number(number) => render_number(number)
  | `Percentage(percentage) => render_number(percentage /. 100.0);

let render_function_rgb = ast => {
  let to_number = percentage => percentage *. 2.55;

  let (colors, alpha) =
    switch (ast) {
    | `Number(`Static_0(colors, alpha))
    | `Number(`Static_1(colors, alpha)) => (colors, alpha)
    | `Percentage(`Static_0(colors, alpha))
    | `Percentage(`Static_1(colors, alpha)) => (
        colors |> List.map(to_number),
        alpha,
      )
    };
  let (red, green, blue) =
    switch (colors) {
    | [red, green, blue] => (red, green, blue)
    | _ => failwith("unreachable")
    };
  let alpha =
    switch (alpha) {
    | Some(((), alpha)) => Some(alpha)
    | None => None
    };

  // TODO: bs-css rgb(float, float, float)
  let red = render_integer(red |> int_of_float);
  let green = render_integer(green |> int_of_float);
  let blue = render_integer(blue |> int_of_float);
  let alpha = Option.map(render_color_alpha, alpha);

  switch (alpha) {
  | Some(alpha) =>
    id([%expr `rgba(([%e red], [%e green], [%e blue], [%e alpha]))])
  | None => id([%expr `rgb(([%e red], [%e green], [%e blue]))])
  };
};
let render_function_hsl = ((hue, saturation, lightness, alpha)) => {
  let hue =
    switch (hue) {
    | `Angle(angle) => angle
    | `Number(degs) => `Deg(degs)
    };

  let hue = render_angle(hue);
  let saturation = render_percentage(saturation);
  let lightness = render_percentage(lightness);
  let alpha =
    Option.map((((), alpha)) => render_color_alpha(alpha), alpha);

  switch (alpha) {
  | Some(alpha) =>
    id(
      [%expr `hsla(([%e hue], [%e saturation], [%e lightness], [%e alpha]))],
    )
  | None => id([%expr `hsl(([%e hue], [%e saturation], [%e lightness]))])
  };
};
let color =
  apply(
    property_color,
    fun
    | `Hex_color(hex) => id([%expr `hex([%e render_string(hex)])])
    | `Named_color(color) => render_named_color(color)
    | `Currentcolor => variants_to_expression(`Currentcolor)
    | `Transparent => variants_to_expression(`Transparent)
    | `Function_rgb(`Rgb(rgb))
    | `Function_rgb(`Rgba(rgb)) => render_function_rgb(rgb)
    | `Function_hsl(hsl) => render_function_hsl(hsl)
    | `Function_hwb(_)
    | `Function_lab(_)
    | `Function_lch(_)
    | `Function_color(_)
    | `Function_device_cmyk(_) => raise(Unsupported_feature),
    [%expr Css.color],
  );
let opacity =
  apply(
    property_opacity,
    fun
    | `Number(number) =>
      string_of_float(number) |> Const.float |> Exp.constant
    | `Percentage(number) =>
      string_of_float(number /. 100.0) |> Const.float |> Exp.constant,
    [%expr Css.opacity],
  );

// css-images-4
let render_position = position => {
  let pos_to_percentage_offset =
    fun
    | `Left
    | `Top => 0.
    | `Right
    | `Bottom => 100.
    | `Center => 50.;
  let to_value =
    fun
    | `Position(pos) => variants_to_expression(pos)
    | `Length(length) => render_length(length)
    | `Percentage(percentage) => render_percentage(percentage);

  let horizontal =
    switch (position) {
    | `Or(Some(pos), _) => (pos, `Zero)
    | `Or(None, _) => (`Center, `Zero)
    | `Static(`Length_percentage(offset), _) => (`Left, offset)
    | `Static((`Center | `Left | `Right) as pos, _) => (pos, `Zero)
    | `And((pos, offset), _) => (pos, offset)
    };
  let horizontal =
    switch (horizontal) {
    | (`Left, `Length(length)) => `Length(length)
    | (_, `Length(_)) => raise(Unsupported_feature)
    | (pos, `Zero) => `Position(pos)
    | (pos, `Percentage(percentage)) =>
      `Percentage(percentage +. pos_to_percentage_offset(pos))
    };
  let horizontal = to_value(horizontal);

  let vertical =
    switch (position) {
    | `Or(_, Some(pos)) => (pos, `Zero)
    | `Or(_, None) => (`Center, `Zero)
    | `Static(_, None) => (`Center, `Zero)
    | `Static(_, Some(`Length_percentage(offset))) => (`Top, offset)
    | `Static(_, Some((`Center | `Bottom | `Top) as pos)) => (pos, `Zero)
    | `And(_, (pos, offset)) => (pos, offset)
    };
  let vertical =
    switch (vertical) {
    | (`Top, `Length(length)) => `Length(length)
    | (_, `Length(_)) => raise(Unsupported_feature)
    | (pos, `Zero) => `Position(pos)
    | (pos, `Percentage(percentage)) =>
      `Percentage(percentage +. pos_to_percentage_offset(pos))
    };
  let vertical = to_value(vertical);

  id([%expr `hv(([%e horizontal], [%e vertical]))]);
};

let object_fit =
  apply(
    property_object_fit,
    fun
    | (`Fill | `None) as variant => variants_to_expression(variant)
    | `Or(_) => raise(Unsupported_feature),
    [%expr Css.objectFit],
  );
let object_position =
  apply(
    property_object_position,
    render_position,
    [%expr Css.objectPosition],
  );
let image_resolution = unsupported(property_image_resolution);
let image_orientation = unsupported(property_image_orientation);
let image_rendering = unsupported(property_image_rendering);

// css-overflow-3
// TODO: maybe implement using strings?
let overflow_x =
  apply(
    property_overflow_x,
    fun
    | `Clip => raise(Unsupported_feature)
    | otherwise => variants_to_expression(otherwise),
    [%expr Css.overflowX],
  );
let overflow_y = variants(property_overflow_y, [%expr Css.overflowY]);
let overflow =
  transform(
    property_overflow,
    List.map(apply_value(overflow_x.value_of_ast)),
    fun
    | [all] => [[%expr Css.overflow([%e all])]]
    | [x, y] =>
      List.concat([
        overflow_x.value_to_expr(x),
        overflow_y.value_to_expr(y),
      ])
    | _ => failwith("unreachable"),
  );
let overflow_clip_margin = unsupported(property_overflow_clip_margin);
let overflow_inline = unsupported(property_overflow_inline);
let text_overflow =
  variants(property_text_overflow, [%expr Css.textOverflow]);
let block_ellipsis = unsupported(property_block_ellipsis);
let max_lines = unsupported(property_max_lines);
let continue = unsupported(property_continue);

// css-text-3
let text_transform =
  apply(
    property_text_transform,
    fun
    | `None => variants_to_expression(`None)
    | `Or(Some(value), None, None) => variants_to_expression(value)
    | `Or(_, Some(_), _)
    | `Or(_, _, Some(_)) => raise(Unsupported_feature)
    | `Or(None, None, None) => failwith("unrecheable"),
    [%expr Css.textTransform],
  );
let white_space = variants(property_white_space, [%expr Css.whiteSpace]);
let tab_size = unsupported(property_tab_size);
let word_break = variants(property_word_break, [%expr Css.wordBreak]);
let line_break = unsupported(property_line_break);
let hyphens = unsupported(property_hyphens);
let overflow_wrap =
  variants(property_overflow_wrap, [%expr Css.overflowWrap]);
let word_wrap = variants(property_word_wrap, [%expr Css.wordWrap]);
let text_align = variants(property_text_align, [%expr Css.textAlign]);
let text_align_all = unsupported(property_text_align_all);
let text_align_last = unsupported(property_text_align_last);
let text_justify = unsupported(property_text_justify);
let word_spacing =
  apply(
    property_word_spacing,
    fun
    | `Normal => variants_to_expression(`Normal)
    | `Length(l) => render_length(l),
    [%expr Css.wordSpacing],
  );
let letter_spacing =
  apply(
    property_word_spacing,
    fun
    | `Normal => variants_to_expression(`Normal)
    | `Length(l) => render_length(l),
    [%expr Css.letterSpacing],
  );
let text_indent =
  apply(
    property_text_indent,
    fun
    | (lp, None, None) => render_length_percentage(lp)
    | _ => raise(Unsupported_feature),
    [%expr Css.textIndent],
  );
let hanging_punctuation = unsupported(property_hanging_punctuation);

// css-flexbox-1
// using id() because refmt
let flex_direction =
  variants(property_flex_direction, [%expr Css.flexDirection]);
let flex_wrap = variants(property_flex_wrap, [%expr Css.flexWrap]);

// shorthand - https://drafts.csswg.org/css-flexbox-1/#flex-flow-property
let flex_flow =
  transform(
    property_flex_flow,
    id,
    ((direction_ast, wrap_ast)) => {
      let direction =
        Option.map(
          ast => flex_direction.ast_to_expr(`Value(ast)),
          direction_ast,
        );
      let wrap =
        Option.map(ast => flex_wrap.ast_to_expr(`Value(ast)), wrap_ast);
      [direction, wrap] |> List.concat_map(Option.value(~default=[]));
    },
  );
// TODO: this is safe?
let order = apply(property_order, render_integer, [%expr Css.order]);
let flex_grow =
  apply(property_flex_grow, render_number, [%expr Css.flexGrow]);
let flex_shrink =
  apply(property_flex_shrink, render_number, [%expr Css.flexShrink]);
// TODO: is safe to just return Css.width when flex_basis?
let flex_basis =
  apply(
    property_flex_basis,
    fun
    | `Content => variants_to_expression(`Content)
    | `Property_width(value_width) =>
      width.value_of_ast(`Value(value_width)),
    [%expr Css.flexBasis],
  );
// TODO: this is incomplete
let flex =
  transform(
    property_flex,
    id,
    fun
    | `None => [[%expr Css.flex(`none)]]
    | `Or(grow_shrink, basis) => {
        let grow_shrink =
          switch (grow_shrink) {
          | None => []
          | Some((grow, shrink)) =>
            List.concat([
              flex_grow.ast_to_expr(`Value(grow)),
              Option.map(
                ast => flex_shrink.ast_to_expr(`Value(ast)),
                shrink,
              )
              |> Option.value(~default=[]),
            ])
          };
        let basis =
          switch (basis) {
          | None => []
          | Some(basis) => flex_basis.ast_to_expr(`Value(basis))
          };
        List.concat([grow_shrink, basis]);
      },
  );
// TODO: justify_content, align_items, align_self, align_content are only for flex, missing the css-align-3 at parser
let justify_content =
  variants(property_justify_content, [%expr Css.justifyContent]);
let align_items = variants(property_align_items, [%expr Css.alignItems]);
let align_self = variants(property_align_self, [%expr Css.alignSelf]);
let align_content =
  variants(property_align_content, [%expr Css.alignContent]);

let found = ({ast_of_string, string_to_expr, _}) => {
  let check_value = string => {
    let.ok _ = ast_of_string(string);
    Ok();
  };
  (check_value, string_to_expr);
};
let properties = [
  // css-sizing-3
  ("width", found(width)),
  ("height", found(height)),
  ("min-width", found(min_width)),
  ("min-height", found(min_height)),
  ("max-width", found(max_width)),
  ("max-height", found(max_height)),
  ("box-sizing", found(box_sizing)),
  ("column-width", found(column_width)),
  // css-box-3
  ("margin-top", found(margin_top)),
  ("margin-right", found(margin_right)),
  ("margin-bottom", found(margin_bottom)),
  ("margin-left", found(margin_left)),
  ("margin", found(margin)),
  ("padding-top", found(padding_top)),
  ("padding-right", found(padding_right)),
  ("padding-bottom", found(padding_bottom)),
  ("padding-left", found(padding_left)),
  ("padding", found(padding)),
  // css-color-4
  ("color", found(color)),
  ("opacity", found(opacity)),
  // css-images-4
  ("object-fit", found(object_fit)),
  ("object-position", found(object_position)),
  ("image-resolution", found(image_resolution)),
  ("image-orientation", found(image_orientation)),
  ("image-rendering", found(image_rendering)),
  // css-overflow-3
  ("overflow-x", found(overflow_x)),
  ("overflow-y", found(overflow_y)),
  ("overflow", found(overflow)),
  ("overflow-clip-margin", found(overflow_clip_margin)),
  ("overflow-inline", found(overflow_inline)),
  ("text-overflow", found(text_overflow)),
  ("block-ellipsis", found(block_ellipsis)),
  ("max-lines", found(max_lines)),
  ("continue", found(continue)),
  // css-box-3
  ("text-transform", found(text_transform)),
  ("white-space", found(white_space)),
  ("tab-size", found(tab_size)),
  ("word-break", found(word_break)),
  ("line-break", found(line_break)),
  ("hyphens", found(hyphens)),
  ("overflow-wrap", found(overflow_wrap)),
  ("word-wrap", found(word_wrap)),
  ("text-align", found(text_align)),
  ("text-align-all", found(text_align_all)),
  ("text-align-last", found(text_align_last)),
  ("text-justify", found(text_justify)),
  ("word-spacing", found(word_spacing)),
  ("letter-spacing", found(letter_spacing)),
  ("text-indent", found(text_indent)),
  ("hanging-punctuation", found(hanging_punctuation)),
  // css-flexbox-1
  ("flex-direction", found(flex_direction)),
  ("flex-wrap", found(flex_wrap)),
  ("flex-flow", found(flex_flow)),
  ("order", found(order)),
  ("flex-grow", found(flex_grow)),
  ("flex-shrink", found(flex_shrink)),
  ("flex-basis", found(flex_basis)),
  ("flex", found(flex)),
  ("justify-content", found(justify_content)),
  ("align-items", found(align_items)),
  ("align-self", found(align_self)),
  ("align-content", found(align_content)),
];

let support_property = name =>
  properties |> List.exists(((key, _)) => key == name);

let render_when_unsupported_features = (name, value) => {
  let to_camel_case = name =>
    name
    |> String.split_on_char('-')
    |> List.map(String.capitalize_ascii)
    |> String.concat("")
    |> String.uncapitalize_ascii;

  let name = to_camel_case(name) |> Const.string |> Exp.constant;
  let value = value |> Const.string |> Exp.constant;

  id([%expr Css.unsafe([%e name], [%e value])]);
};
let parse_declarations = ((name, value)) => {
  let map_parse_error = result =>
    Result.map_error(str => `Invalid_value(str), result);

  let.ok (_, (check_string, string_to_expr)) =
    properties
    |> List.find_opt(((key, _)) => key == name)
    |> Option.to_result(~none=`Not_found);

  switch (render_css_wide_keywords(name, value)) {
  | Ok(value) => Ok(value)
  | Error(_) =>
    let.ok () = check_string(value) |> map_parse_error;
    switch (string_to_expr(value)) {
    | result => map_parse_error(result)
    | exception Unsupported_feature =>
      Ok([render_when_unsupported_features(name, value)])
    };
  };
};
