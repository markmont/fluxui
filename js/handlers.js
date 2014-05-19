// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function moduleDidLoad() {
  common.hideModule();
}

// Called by the common.js module.
function domContentLoaded(name, tc, config, width, height) {
  common.attachDefaultListeners();
  common.createNaClModule(name, tc, config, width, height);
  /*
  navigator.webkitPersistentStorage.requestQuota(1024 * 1024,
      function(bytes) {
        common.updateStatus(
            'Allocated ' + bytes + ' bytes of persistant storage.');
        common.attachDefaultListeners();
        common.createNaClModule(name, tc, config, width, height);
      },
      function(e) { alert('Failed to allocate space') });
  */
}

// Called by the common.js module.
function attachListeners() {
}


// For testing:
function runtest(callback) {
  console.log( "runtest() called" );
  nacl_module.postMessage(makeCall('runtest', callback));
}


/**
 * Return true when |s| starts with the string |prefix|.
 *
 * @param {string} s The string to search.
 * @param {string} prefix The prefix to search for in |s|.
 * @return {boolean} Whether |s| starts with |prefix|.
 */
function startsWith(s, prefix) {
  // indexOf would search the entire string, lastIndexOf(p, 0) only checks at
  // the first index. See: http://stackoverflow.com/a/4579228
  return s.lastIndexOf(prefix, 0) === 0;
}

var nacl_callback_table = { 'id': 0 };

function makeCall(func, callback) {
  var message = func;
  var callback_id = func + nacl_callback_table['id']++;
  nacl_callback_table[callback_id] = callback;
  message += '\1' + callback_id;
  for (var i = 2; i < arguments.length; ++i) {
    message += '\1' + arguments[i];
  }

  //console.log("makeCall(): " + message);
  return message;
}

// Called by the common.js module.
function handleMessage(message_event) {
  var msg = message_event.data;
  if (startsWith(msg, 'Error:')) {
    common.logMessage(msg);
  } else {
    // Result from a function call.
    var params = msg.split('\1');
    var callback_id = params[0];
    var callback = nacl_callback_table[callback_id];
    delete nacl_callback_table[callback_id];

    if (!callback) {
      common.logMessage('Error: Bad message ' + callback_id +
                        ' received from NaCl module.');
      return;
    }

    callback.apply(null, params.slice(1));
  }
}

