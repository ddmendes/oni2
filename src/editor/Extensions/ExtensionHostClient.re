/*
 * ExtensionHostClient.re
 *
 * This is a client-side API for integrating with our VSCode extension host API.
 *
 */

open Oni_Core;
open Reason_jsonrpc;
/* open Revery; */

module Protocol = ExtensionHostProtocol;

type t = {
  process: NodeProcess.t,
  rpc: Rpc.t,
};

let emptyJsonValue = `Assoc([]);

type simpleCallback = unit => unit;
let defaultCallback: simpleCallback = () => ();

type messageHandler =
  (int, Yojson.Safe.json) => result(option(Yojson.Safe.json), string);
let defaultMessageHandler = (_, _) => Ok(None);

let start =
    (
      ~initData=ExtensionHostInitData.create(),
      ~onMessage=defaultMessageHandler,
      ~onClosed=defaultCallback,
      setup: Setup.t,
    ) => {
        print_endline ("got here");
  let args = ["--type=extensionHost"];
  let env = [
    "AMD_ENTRYPOINT=vs/workbench/services/extensions/node/extensionHostProcess",
  ];
  let process =
    NodeProcess.start(~args, ~env, setup, setup.extensionHostPath);

  let lastReqId = ref(0);
  let rpcRef = ref(None);

  let send = (msgType: int, msg: Yojson.Safe.json) => {
      switch (rpcRef^) {
      | None => prerr_endline ("RPC not initialized.");
      | Some(v) => {
          incr(lastReqId);
          let reqId = lastReqId^;

          let request = `Assoc([
            ("type", `Int(msgType)),
            ("reqId", `Int(reqId)),
            ("payload", msg),
          ]);

          print_endline ("Sending request");
          Rpc.sendNotification(v, "ext/msg", request);
      }
      };
  };

  let handleMessage = (id: int, _reqId: int, payload: Yojson.Safe.json) => {
    if (id == Protocol.MessageType.initialized) {
        print_endline ("HANDLE MESSAGE");
        send(Protocol.MessageType.initData, ExtensionHostInitData.to_yojson(initData));
    } else {
        switch (onMessage(id, payload)) {
        | Ok(None) => ()
        | Ok(Some(_)) =>
          /* TODO: Send response */
          ()
        | Error(_) =>
          /* TODO: Send error */
          ()
        };
    }
  };

  let onNotification = (n: Notification.t, _) => {
    switch (n.method, n.params) {
    | ("host/msg", json) =>
      open Protocol.Notification;
      print_endline("[Extension Host Client] Unknown message: " ++ n.method);
      print_endline("JSON: " ++ Yojson.Safe.to_string(json));
      let parsedMessage = Protocol.Notification.of_yojson(json);
      handleMessage(
        parsedMessage.msgType,
        parsedMessage.reqId,
        parsedMessage.payload,
      );
    | _ =>
      print_endline("[Extension Host Client] Unknown message: " ++ n.method)
    };
  };

  let onRequest = (_, _) => Ok(emptyJsonValue);

  let rpc =
    Rpc.start(
      ~onNotification,
      ~onRequest,
      ~onClose=onClosed,
      process.stdout,
      process.stdin,
    );

  rpcRef := Some(rpc);

  {process, rpc };
};

let pump = (v: t) => Rpc.pump(v.rpc);