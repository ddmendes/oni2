open Revery.UI;
open Oni_Core;

module Constants = {
  include Constants;

  let diffMarkersMaxLineCount = 2000;
  let diffMarkerWidth = 3.;
  let gutterMargin = 3.;
};

let renderLineNumber =
    (
      ~context: Draw.context,
      lineNumber: int,
      lineNumberWidth: float,
      colors: Colors.t,
      lineSetting,
      cursorLine: int,
      yOffset: float,
    ) => {
  let isActiveLine = lineNumber == cursorLine;
  let y = yOffset -. context.fontMetrics.ascent;

  let lineNumber =
    string_of_int(
      LineNumber.getLineNumber(
        ~bufferLine=lineNumber + 1,
        ~cursorLine=cursorLine + 1,
        ~setting=lineSetting,
        (),
      ),
    );

  let lineNumberXOffset =
    isActiveLine
      ? 0.
      : lineNumberWidth
        /. 2.
        -. float(String.length(lineNumber))
        *. context.charWidth
        /. 2.;

  let color =
    isActiveLine
      ? colors.lineNumberActiveForeground : colors.lineNumberForeground;

  Draw.utf8Text(~context, ~x=lineNumberXOffset, ~y, ~color, lineNumber);
};

let renderLineNumbers =
    (
      ~context,
      ~lineNumberWidth,
      ~height,
      ~colors: Colors.t,
      ~count,
      ~showLineNumbers,
      ~cursorLine,
    ) => {
  Draw.rect(
    ~context,
    ~x=0.,
    ~y=0.,
    ~width=lineNumberWidth,
    ~height=float(height),
    ~color=colors.gutterBackground,
  );

  Draw.renderImmediate(~context, ~count, (item, offset) =>
    renderLineNumber(
      ~context,
      item,
      lineNumberWidth,
      colors,
      showLineNumbers,
      cursorLine,
      offset,
    )
  );
};

let render =
    (
      ~showLineNumbers,
      ~lineNumberWidth,
      ~width,
      ~height,
      ~colors,
      ~editorFont: Service_Font.font,
      ~scrollY,
      ~lineHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
      canvasContext,
    ) => {
  let context =
    Draw.createContext(
      ~canvasContext,
      ~width,
      ~height,
      ~scrollX=0.,
      ~scrollY,
      ~lineHeight,
      ~editorFont,
    );

  if (showLineNumbers != `Off) {
    renderLineNumbers(
      ~context,
      ~lineNumberWidth,
      ~height,
      ~colors,
      ~count,
      ~showLineNumbers,
      ~cursorLine,
    );
  };

  Option.iter(
    EditorDiffMarkers.render(
      ~scrollY=context.scrollY,
      ~rowHeight=context.lineHeight,
      ~x=lineNumberWidth,
      ~height=float(height),
      ~width=Constants.diffMarkerWidth,
      ~count,
      ~canvasContext,
      ~colors,
    ),
    diffMarkers,
  );
};

let make =
    (
      ~showLineNumbers,
      ~height,
      ~colors,
      ~editorFont: Service_Font.font,
      ~scrollY,
      ~lineHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
      (),
    ) => {
  let lineNumberWidth =
    showLineNumbers != `Off
      ? LineNumber.getLineNumberPixelWidth(
          ~lines=count,
          ~fontPixelWidth=editorFont.measuredWidth,
          (),
        )
      : 0.0;

  let totalWidth =
    lineNumberWidth +. Constants.diffMarkerWidth +. Constants.gutterMargin;

  let style =
    Style.[
      overflow(`Hidden),
      position(`Absolute),
      top(0),
      left(0),
      width(int_of_float(totalWidth)),
      bottom(0),
    ];

  let render =
    render(
      ~showLineNumbers,
      ~lineNumberWidth,
      ~width=int_of_float(totalWidth),
      ~height,
      ~colors,
      ~editorFont,
      ~scrollY,
      ~lineHeight,
      ~count,
      ~cursorLine,
      ~diffMarkers,
    );

  (totalWidth, <Canvas style render />);
};
