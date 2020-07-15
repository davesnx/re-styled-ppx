open Jest;

let supportList = [
  [%css "overflow-x: clip"],
  [%css "opacity: 0.9"],
  /* [%css "box-shadow: 1px 54px 1px blue"], */
  /* [%css "box-shadow: 2px 3px blue"], */
  /* [%css "text-shadow: 0px 0px blue"], */
  /* [%css "text-shadow: 10px 0px 0px blue"], */
  /* [%css {| overflow-y: visible; overflow-x: visible; overflow: hidden; |}], */
  /* [%css "visibility: visible"], */
  /* [%css "hyphens: manual"], */
  /* [%css "stroke: none"], */
  /* [%css "order: 0"], */
  /* [%css "direction: ltr"], */
  /* [%css "content: normal"], */
  /* [%css "clear: none"], */
  [%css "box-sizing: content-box"],
  [%css "box-sizing: border-box"],
  /* [%css "box-shadow: none"], */
  /* [%css "border-collapse: separate"], */
  /* [%css "transition-property: all"], */
  /* [%css "transition-duration: 0.5s"], */
  /* [%css "transition-timing-function: ease"], */
  /* [%css "transition-timing-function: step-end"], */
  /* [%css "transition-delay: 0.5s"], */
  /* [%css "transition: none;"], */
  /* [%css "transition: ease 250ms"], */
  /* [%css "transition: ease 250ms"], */
  /* [%css "transition: margin-left 4s ease-in-out 1s"], */
  /* [%css "transition: width 2s, height 2s, background-color 2s, transform 2s"], */
  /* [%css "animation-name: slidein"], */
  /* [%css "animation-duration: 3s"], */
  /* [%css "animation-timing-function: ease"], */
  /* [%css "animation-delay: 3s"], */
  /* [%css "animation-direction: alternate"], */
  /* [%css "animation-iteration-count: infinite"], */
  /* [%css "animation-iteration-count: 1"], */
  /* [%css "animation-iteration-count: 2, 1, 5"], */
  /* [%css "animation-fill-mode: backwards"], */
  /* [%css "animation-play-state: "], */
  /* [%css "animation: 3s infinite alternate slidein"], */
  /* [%css "transform: translate(10px, 10px)"], */
  /* [%css "transform: translateX(10px) rotate(10deg) translateY(5px)"], */
  /* [%css "transform: matrix(1.0, 2.0, 3.0, 4.0, 5.0, 6.0)" */
  /* [%css "transform: translate(12px, 50%)" */
  /* [%css "transform: translateX(2em)" */
  /* [%css "transform: translateY(3in)" */
  /* [%css "transform: scale(2, 0.5)" */
  /* [%css "transform: scaleX(2)" */
  /* [%css "transform: scaleY(0.5)" */
  /* [%css "transform: rotate(0.5turn)" */
  /* [%css "transform: skew(30deg, 20deg)" */
  /* [%css "transform: skewX(30deg)" */
  /* [%css "transform: skewY(1.07rad)" */
  /* [%css "transform: matrix3d(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0)" */
  /* [%css "transform: translate3d(12px, 50%, 3em)" */
  /* [%css "transform: translateZ(2px)" */
  /* [%css "transform: scale3d(2.5, 1.2, 0.3)" */
  /* [%css "transform: scaleZ(0.3)" */
  /* [%css "transform: rotate3d(1, 2.0, 3.0, 10deg)" */
  /* [%css "transform: rotateX(10deg)" */
  /* [%css "transform: rotateY(10deg)" */
  /* [%css "transform: rotateZ(10deg)" */
  /* [%css "transform: perspective(17px)" */
  /* [%css "font-family: 'Open Sans', '-system', sans-serif"], */
  [%css "transform: initial"],
  [%css "flex-flow: row wrap"],
  [%css "flex: 1 2 content"],
  [%css "flex: unset"],
  // css-sizing-3
  [%css "width: auto"],
  [%css "width: 0"],
  [%css "height: 5px"],
  [%css "min-width: 5%"],
  [%css "min-height: 5em"],
  [%css "max-width: none"],
  [%css "max-height: 3vh"],
  [%css "box-sizing: border-box"],
  // css-box-3
  [%css "margin-top: auto"],
  [%css "margin-right: 1px"],
  [%css "margin-bottom: 2px"],
  [%css "margin-left: 3px"],
  [%css "margin: 1px"],
  [%css "margin: 1px 2px"],
  [%css "margin: 1px 2px 3px"],
  [%css "margin: 1px 2px 3px 4px"],
  [%css "padding-top: 0"],
  [%css "padding-right: 1px"],
  [%css "padding-bottom: 2px"],
  [%css "padding-left: 3px"],
  [%css "padding: 1px"],
  [%css "padding: 1px 2px"],
  [%css "padding: 1px 2px 3px"],
  [%css "padding: 1px 2px 3px 4px"],
  // css-color-4
  [%css "color: #012"],
  [%css "color: #0123"],
  [%css "color: #012345"],
  [%css "color: #01234567"],
  [%css "color: blue"],
  [%css "color: currentcolor"],
  [%css "color: transparent"],
  [%css "color: rgb(1 2 3)"],
  [%css "color: rgb(1 2 3 / .4)"],
  [%css "color: rgba(1, 2, 3)"],
  [%css "color: rgba(1, 2, 3, .4)"],
  [%css "color: hsl(120deg 100% 50%)"],
  [%css "opacity: 0.5"],
  [%css "opacity: 60%"],
  // css-images-4
  [%css "object-fit: fill"],
  [%css "object-position: right bottom"],
  // css-overflow-3
  [%css "overflow-x: auto"],
  [%css "overflow-y: hidden"],
  [%css "overflow: scroll"],
  [%css "overflow: scroll visible"],
  [%css "text-overflow: clip"],
  [%css "text-overflow: ellipsis"],
  // css-text-3
  [%css "text-transform: capitalize"],
  [%css "white-space: break-spaces"],
  [%css "word-break: keep-all"],
  [%css "overflow-wrap: anywhere"],
  [%css "word-wrap: normal"],
  // [%css "text-align: start"],
  [%css "text-align: left"],
  [%css "word-spacing: normal"],
  [%css "word-spacing: 5px"],
  [%css "letter-spacing: normal"],
  [%css "letter-spacing: 5px"],
  [%css "text-indent: 5%"],
];

Belt.List.forEachWithIndex(supportList, (index, css) => {
  test("Component " ++ string_of_int(index) ++ " renders", () => {
    css |> Expect.expect |> Expect.toMatchSnapshot
  })
});
