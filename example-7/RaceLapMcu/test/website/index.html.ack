<!DOCTYPE html>
<html>

<head>
  <title>BLE File Transfer Example</title>
</head>

<body>
  <div>
    Test file transfer over BLE. To use:
    <ul>
      <li>Flash an Arduino Nano BLE Sense board with <a href="https://github.com/petewarden/ble_file_transfer">the test
          sketch</a>.</li>
      <li>Click the connect button and pair with the board.</li>
      <li>Press the transfer file button.</li>
    </ul>
    You should see the bytes remaining count down, and then a 'File transfer succeeded' message at the end.
  </div>
  <div>
    <button id="connect-button">Connect</button>
    <button id="listdir-button">listdir</button>
    <button id="delfile-button">delfile</button>
    <button id="downloadfile-button">downloadfile</button>
    <button id="battery-button">baseinfo</button>
    <button id="trackinfo-button">trackinfo</button>
    <button id="poweroff-button">poweroff</button>
    <button id="cancel-transfer-button">Cancel Transfer</button>
    <div id="status-label">Click button to connect to the board</div>
  </div>
</body>
<script type="text/javascript">
  /*! crc32.js (C) 2014-present SheetJS -- http://sheetjs.com */
  /* vim: set ts=2: */
  /*exported CRC32 */
  var CRC32;
  (function (factory) {
    /*jshint ignore:start */
    /*eslint-disable */
    if (typeof DO_NOT_EXPORT_CRC === 'undefined') {
      if ('object' === typeof exports) {
        factory(exports);
      } else if ('function' === typeof define && define.amd) {
        define(function () {
          var module = {};
          factory(module);
          return module;
        });
      } else {
        factory(CRC32 = {});
      }
    } else {
      factory(CRC32 = {});
    }
    /*eslint-enable */
    /*jshint ignore:end */
  }(function (CRC32) {
    CRC32.version = '1.2.3';
    /*global Int32Array */
    function signed_crc_table() {
      var c = 0, table = new Array(256);

      for (var n = 0; n != 256; ++n) {
        c = n;
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        c = ((c & 1) ? (-306674912 ^ (c >>> 1)) : (c >>> 1));
        table[n] = c;
      }

      return typeof Int32Array !== 'undefined' ? new Int32Array(table) : table;
    }

    var T0 = signed_crc_table();
    function slice_by_16_tables(T) {
      var c = 0, v = 0, n = 0, table = typeof Int32Array !== 'undefined' ? new Int32Array(4096) : new Array(4096);

      for (n = 0; n != 256; ++n) table[n] = T[n];
      for (n = 0; n != 256; ++n) {
        v = T[n];
        for (c = 256 + n; c < 4096; c += 256) v = table[c] = (v >>> 8) ^ T[v & 0xFF];
      }
      var out = [];
      for (n = 1; n != 16; ++n) out[n - 1] = typeof Int32Array !== 'undefined' ? table.subarray(n * 256, n * 256 + 256) : table.slice(n * 256, n * 256 + 256);
      return out;
    }
    var TT = slice_by_16_tables(T0);
    var T1 = TT[0], T2 = TT[1], T3 = TT[2], T4 = TT[3], T5 = TT[4];
    var T6 = TT[5], T7 = TT[6], T8 = TT[7], T9 = TT[8], Ta = TT[9];
    var Tb = TT[10], Tc = TT[11], Td = TT[12], Te = TT[13], Tf = TT[14];
    function crc32_bstr(bstr, seed) {
      var C = seed ^ -1;
      for (var i = 0, L = bstr.length; i < L;) C = (C >>> 8) ^ T0[(C ^ bstr.charCodeAt(i++)) & 0xFF];
      return ~C;
    }

    function crc32_buf(B, seed) {
      var C = seed ^ -1, L = B.length - 15, i = 0;
      for (; i < L;) C =
        Tf[B[i++] ^ (C & 255)] ^
        Te[B[i++] ^ ((C >> 8) & 255)] ^
        Td[B[i++] ^ ((C >> 16) & 255)] ^
        Tc[B[i++] ^ (C >>> 24)] ^
        Tb[B[i++]] ^ Ta[B[i++]] ^ T9[B[i++]] ^ T8[B[i++]] ^
        T7[B[i++]] ^ T6[B[i++]] ^ T5[B[i++]] ^ T4[B[i++]] ^
        T3[B[i++]] ^ T2[B[i++]] ^ T1[B[i++]] ^ T0[B[i++]];
      L += 15;
      while (i < L) C = (C >>> 8) ^ T0[(C ^ B[i++]) & 0xFF];
      return ~C;
    }

    function crc32_str(str, seed) {
      var C = seed ^ -1;
      for (var i = 0, L = str.length, c = 0, d = 0; i < L;) {
        c = str.charCodeAt(i++);
        if (c < 0x80) {
          C = (C >>> 8) ^ T0[(C ^ c) & 0xFF];
        } else if (c < 0x800) {
          C = (C >>> 8) ^ T0[(C ^ (192 | ((c >> 6) & 31))) & 0xFF];
          C = (C >>> 8) ^ T0[(C ^ (128 | (c & 63))) & 0xFF];
        } else if (c >= 0xD800 && c < 0xE000) {
          c = (c & 1023) + 64; d = str.charCodeAt(i++) & 1023;
          C = (C >>> 8) ^ T0[(C ^ (240 | ((c >> 8) & 7))) & 0xFF];
          C = (C >>> 8) ^ T0[(C ^ (128 | ((c >> 2) & 63))) & 0xFF];
          C = (C >>> 8) ^ T0[(C ^ (128 | ((d >> 6) & 15) | ((c & 3) << 4))) & 0xFF];
          C = (C >>> 8) ^ T0[(C ^ (128 | (d & 63))) & 0xFF];
        } else {
          C = (C >>> 8) ^ T0[(C ^ (224 | ((c >> 12) & 15))) & 0xFF];
          C = (C >>> 8) ^ T0[(C ^ (128 | ((c >> 6) & 63))) & 0xFF];
          C = (C >>> 8) ^ T0[(C ^ (128 | (c & 63))) & 0xFF];
        }
      }
      return ~C;
    }
    CRC32.table = T0;
    // $FlowIgnore
    CRC32.bstr = crc32_bstr;
    // $FlowIgnore
    CRC32.buf = crc32_buf;
    // $FlowIgnore
    CRC32.str = crc32_str;
  }));


  !function (globals) {
    'use strict'

    var convertString = {
      bytesToString: function (bytes) {
        return bytes.map(function (x) { return String.fromCharCode(x) }).join('')
      },
      stringToBytes: function (str) {
        return str.split('').map(function (x) { return x.charCodeAt(0) })
      }
    }

    //http://hossa.in/2012/07/20/utf-8-in-javascript.html
    convertString.UTF8 = {
      bytesToString: function (bytes) {
        return decodeURIComponent(escape(convertString.bytesToString(bytes)))
      },
      stringToBytes: function (str) {
        return convertString.stringToBytes(unescape(encodeURIComponent(str)))
      }
    }

    if (typeof module !== 'undefined' && module.exports) { //CommonJS
      module.exports = convertString
    } else {
      globals.convertString = convertString
    }

  }(this);


  let trackdata = {
    "trackname": "liwand1mini",
    "trackplan": [
      [
        31.934493,
        118.986260,
        31.934659,
        118.986156
      ],
      [
        31.935279,
        118.986374,
        31.935097,
        118.986298
      ],
      [
        31.934911,
        118.985879,
        31.934865,
        118.986073
      ],
      [
        31.934722,
        118.985327,
        31.934918,
        118.985345
      ],
      [
        31.935798,
        118.986160,
        31.935994,
        118.986176
      ],
      [
        31.935468,
        118.986605,
        31.935468,
        118.986803
      ],
      [
        31.934884,
        118.987121,
        31.934764,
        118.987279
      ],
      [
        31.934679,
        118.986682,
        31.934449,
        118.986870
      ]
    ]
  }
  const SERVICE_UUID = '6182d488-0000-4889-bb3d-d90c8e351edd';




  const pCharacteristicCMD = '6182d488-0001-4889-bb3d-d90c8e351edd';
  const pCharacteristicTrackInfo = '6182d488-0002-4889-bb3d-d90c8e351edd';
  const pCharacteristicListDir = '6182d488-0003-4889-bb3d-d90c8e351edd';
  const pCharacteristicDownloadFile = '6182d488-0004-4889-bb3d-d90c8e351edd';



  const FILE_LENGTH_UUID = 'bf88b656-3001-4a61-86e0-769c741026c0';
  const FILE_MAXIMUM_LENGTH_UUID = 'bf88b656-3002-4a61-86e0-769c741026c0';
  const FILE_CHECKSUM_UUID = 'bf88b656-3003-4a61-86e0-769c741026c0';
  const COMMAND_UUID = 'bf88b656-3004-4a61-86e0-769c741026c0';
  const TRANSFER_STATUS_UUID = 'bf88b656-3005-4a61-86e0-769c741026c0';
  const ERROR_MESSAGE_UUID = 'bf88b656-3006-4a61-86e0-769c741026c0';

  const connectButton = document.getElementById('connect-button');
  const listdirButton = document.getElementById('listdir-button');
  const delfileButton = document.getElementById('delfile-button');
  const downloadfileButton = document.getElementById('downloadfile-button');
  const batteryButton = document.getElementById('battery-button');
  const trackinfoButton = document.getElementById('trackinfo-button');
  const poweroffButton = document.getElementById('poweroff-button');




  const transferFileButton = document.getElementById('transfer-file-button');
  const cancelTransferButton = document.getElementById('cancel-transfer-button');
  const statusElement = document.getElementById('status-label');

  // Check that the browser supports WebBLE, and raise a warning if not.
  if (!("bluetooth" in navigator)) {
    msg('Browser not supported');
    alert('Error: This browser doesn\'t support Web Bluetooth. Try using Chrome.');
  }

  // This is a simple demonstration of how to use WebBLE to transfer tens of kilobytes
  // of data to an Arduino Nano BLE board with the corresponding sketch installed.
  //
  // The basic API is that you call connect() to prompt the user to pair with the board,
  // then transferFile() with the data you want to send. If it completes successfully
  // then onTransferSuccess() will be called, otherwise onTransferError() will be
  // invoked. Progress information is reported by the msg() function, and there's a
  // human-readable explanation of any error in onErrorMessageChanged().
  //
  // The file transfer process is verified using a CRC32 checksum, which you can also
  // choose to read before sending a file if you want to tell if it's already been sent.

  connectButton.addEventListener('click', function (event) {
    connect();
    listdirButton.addEventListener('click', function (event) {
      msg('list dir ...');
      // You'll want to replace this with the data you want to transfer.
      // let fileContents = prepareDummyFileContents(30 * 1024);
      // transferFile(fileContents);

      // senddata();
      sendcmd("listdir");
    });

    delfileButton.addEventListener('click', function (event) {
      msg('delfile ...');
      // You'll want to replace this with the data you want to transfer.
      // let fileContents = prepareDummyFileContents(30 * 1024);
      // transferFile(fileContents);

      // senddata();

      let fname = prompt("filename");
      if (fname) sendcmd("delfile", fname);
    });

    downloadfileButton.addEventListener('click', function (event) {
      msg('downloadfile ...');
      // You'll want to replace this with the data you want to transfer.
      // let fileContents = prepareDummyFileContents(30 * 1024);
      // transferFile(fileContents);

      // senddata();

      let fname = prompt("filename");
      console.log(fname);
      if (fname) sendcmd("downloadfile", fname);
    });

    batteryButton.addEventListener('click', function (event) {
      msg('baseinfoButton ...');
      sendcmd("baseinfo", "");
    });

    trackinfoButton.addEventListener('click', function (event) {
      msg('trackinfoButton ...');
      sendcmd("trackinfo", "");
    });

    poweroffButton.addEventListener('click', function (event) {
      msg('poweroffButton ...');
      sendcmd("poweroff", "");
    });
    cancelTransferButton.addEventListener('click', function (event) {
      msg('Trying to cancel transfer ...');
      closeble();
    });
  });

  // ------------------------------------------------------------------------------
  // This section contains functions you may want to customize for your own page.

  // You'll want to replace these two functions with your own logic, to take what
  // actions your application needs when a file transfer succeeds, or errors out.
  async function onTransferSuccess() {
    isFileTransferInProgress = false;
    let checksumValue = await fileChecksumCharacteristic.readValue();
    let checksumArray = new Uint32Array(checksumValue.buffer);
    let checksum = checksumArray[0];
    msg('File transfer succeeded: Checksum 0x' + checksum.toString(16));
  }

  // Called when something has gone wrong with a file transfer.
  function onTransferError() {
    isFileTransferInProgress = false;
    msg("File transfer error");
  }

  // Called when an error message is received from the device. This describes what
  // went wrong with the transfer in a user-readable form.
  function onErrorMessageChanged(event) {
    let value = new Uint8Array(event.target.value.buffer);
    let utf8Decoder = new TextDecoder();
    let errorMessage = utf8Decoder.decode(value);
    console.log("Error message = " + errorMessage);
  }

  // Display logging information in the interface, you'll want to customize this
  // for your page.
  function msg(m) {
    console.log("msg->", m);
    statusElement.innerHTML = m;
  }


  var crc32str = "";
  var file_count = 0;

  function onDownloadFile(event) {
    // // let value = new Uint32Array(event.target.value.buffer);
    // // let statusCode = value[0];
    // const encoder = new TextEncoder();
    // const array = encoder.encode("€"); // Uint8Array(3) [226, 130, 172]

    const decoder = new TextDecoder();
    const encoder = new TextEncoder('utf-8');

    // let str = decoder.decode(array); // String "€"


    //console.log("event", event)

    let typedArray = new Uint8Array(event.target.value.buffer);
    //let str = decoder.decode(event.target.value);
    let str = convertString.UTF8.bytesToString(Array.from(typedArray));

    console.log("onDownloadFile:", str);


    //console.log("str-->", str);

    if (str.indexOf(":start") > -1) {
      CharacteristicDownloadfile.writeValue(encoder.encode(file_count++));
    } else if (str.indexOf(":crc32") > -1) {
      console.log(CRC32.str(crc32str) >>> 0);               //  2647669026 (unsigned)
      console.log((CRC32.str(crc32str) >>> 0).toString(16));

    } else {
      crc32str += str;
      CharacteristicDownloadfile.writeValue(encoder.encode(file_count++));
      // crc32str = CRC32.str(str, crc32str);
    }

    // let str = event.target.value.buffer;
    // console.log(encoder.decode(str));
  }

  function onCMD(event) {
    // // let value = new Uint32Array(event.target.value.buffer);
    // // let statusCode = value[0];
    // const encoder = new TextEncoder();
    // const array = encoder.encode("€"); // Uint8Array(3) [226, 130, 172]

    const decoder = new TextDecoder();
    // let str = decoder.decode(array); // String "€"


    // console.log("str", str)

    let cmd = decoder.decode(event.target.value);

    console.log("OnCMD:", cmd);




    // let str = event.target.value.buffer;
    // console.log(encoder.decode(str));
  }



  function onListDir(event) {
    // // let value = new Uint32Array(event.target.value.buffer);
    // // let statusCode = value[0];
    // const encoder = new TextEncoder();
    // const array = encoder.encode("€"); // Uint8Array(3) [226, 130, 172]

    const decoder = new TextDecoder();
    // let str = decoder.decode(array); // String "€"


    // console.log("str", str)

    console.log("listdir", decoder.decode(event.target.value));
    // let str = event.target.value.buffer;
    // console.log(encoder.decode(str));
  }

  async function sendcmd(_cmd, _params) {
    let encoder = new TextEncoder('utf-8');

    let cmd = {
      cmd: _cmd,
      params: _params,
    }

    console.log(JSON.stringify(cmd))
    // CharacteristicCMD.writeValue(encoder.encode(JSON.stringify(trackdata)));
    CharacteristicCMD.writeValue(encoder.encode(JSON.stringify(cmd)));
  }


  async function senddata() {
    let encoder = new TextEncoder('utf-8');



    CharacteristicCMD.writeValue(encoder.encode(JSON.stringify(trackdata)));
  }


  async function closeble() {
    device.gatt.onDisconnected
  }
  // ------------------------------------------------------------------------------
  // This section has the public APIs for the transfer process, which you
  // shouldn't need to modify but will have to call.

  async function connect() {
    msg('Requesting device ...');

    const device = await navigator.bluetooth.requestDevice({
      // filters: [
      //   { services: [SERVICE_UUID] }]
      //  acceptAllDevices: true,
      filters: [
        { services: [SERVICE_UUID] },
        // { name: "XLAPBLE01" },
        // { namePrefix: "Prefix" },
      ],

    });

    msg('Connecting to device ...');
    function onDisconnected(event) {
      msg('Device ' + device.name + ' is disconnected.');
    }
    device.addEventListener('gattserverdisconnected', onDisconnected);

    // const gatt = device.gatt;
    const server = await device.gatt.connect();

    msg('Getting primary service ...');
    const service = await server.getPrimaryService(SERVICE_UUID);

    msg('Getting characteristics ...');

    CharacteristicCMD = await service.getCharacteristic(pCharacteristicCMD);
    await CharacteristicCMD.startNotifications();
    CharacteristicCMD.addEventListener('characteristicvaluechanged', onCMD);
    msg('Getting onCMD ...');

    CharacteristicListDir = await service.getCharacteristic(pCharacteristicListDir)
    msg('Getting CharacteristicListDir ...');
    await CharacteristicListDir.startNotifications();
    msg('Getting CharacteristicListDir startNotifications ...');
    CharacteristicListDir.addEventListener('characteristicvaluechanged', onListDir);
    msg('Getting onListDir ...');

    CharacteristicDownloadfile = await service.getCharacteristic(pCharacteristicDownloadFile);
    await CharacteristicDownloadfile.startNotifications();
    CharacteristicDownloadfile.addEventListener('characteristicvaluechanged', onDownloadFile);
    msg('Getting onCMD ...');



    // CharacteristicTrackInfo = await service.getCharacteristic(pCharacteristicTrackInfo);

    // CharacteristicListDir = await service.getCharacteristic(pCharacteristicListDir);

    // fileBlockCharacteristic = await service.getCharacteristic(FILE_BLOCK_UUID);
    // fileLengthCharacteristic = await service.getCharacteristic(FILE_LENGTH_UUID);
    // fileMaximumLengthCharacteristic = await service.getCharacteristic(FILE_MAXIMUM_LENGTH_UUID);
    // fileChecksumCharacteristic = await service.getCharacteristic(FILE_CHECKSUM_UUID);
    // commandCharacteristic = await service.getCharacteristic(COMMAND_UUID);
    // transferStatusCharacteristic = await service.getCharacteristic(TRANSFER_STATUS_UUID);
    // await transferStatusCharacteristic.startNotifications();
    // transferStatusCharacteristic.addEventListener('characteristicvaluechanged', onTransferStatusChanged);
    // errorMessageCharacteristic = await service.getCharacteristic(ERROR_MESSAGE_UUID);
    // await errorMessageCharacteristic.startNotifications();
    // errorMessageCharacteristic.addEventListener('characteristicvaluechanged', onErrorMessageChanged);

    // isFileTransferInProgress = false;

    msg('Connected to device');
  }

  async function transferFile(fileContents) {
    let maximumLengthValue = await fileMaximumLengthCharacteristic.readValue();
    let maximumLengthArray = new Uint32Array(maximumLengthValue.buffer);
    let maximumLength = maximumLengthArray[0];
    if (fileContents.byteLength > maximumLength) {
      msg("File length is too long: " + fileContents.byteLength + " bytes but maximum is " + maximumLength);
      return;
    }

    if (isFileTransferInProgress) {
      msg("Another file transfer is already in progress");
      return;
    }

    let fileLengthArray = Int32Array.of(fileContents.byteLength);
    await fileLengthCharacteristic.writeValue(fileLengthArray);
    let fileChecksum = crc32(fileContents);
    let fileChecksumArray = Uint32Array.of(fileChecksum);
    await fileChecksumCharacteristic.writeValue(fileChecksumArray);

    let commandArray = Int32Array.of(1);
    await commandCharacteristic.writeValue(commandArray);

    sendFileBlock(fileContents, 0);
  }

  async function cancelTransfer() {
    let commandArray = Int32Array.of(2);
    await commandCharacteristic.writeValue(commandArray);
  }

  // ------------------------------------------------------------------------------
  // The rest of these functions are internal implementation details, and shouldn't
  // be called by users of this module.

  function onTransferInProgress() {
    isFileTransferInProgress = true;
  }



  function onTransferStatusChanged(event) {
    let value = new Uint32Array(event.target.value.buffer);
    let statusCode = value[0];
    if (statusCode === 0) {
      onTransferSuccess();
    } else if (statusCode === 1) {
      onTransferError();
    } else if (statusCode === 2) {
      onTransferInProgress();
    }
  }

  function prepareDummyFileContents(fileLength) {
    let result = new ArrayBuffer(fileLength);
    let bytes = new Uint8Array(result);
    const contentString = 'The quick brown fox jumped over the lazy dog. ';
    for (var i = 0; i < bytes.length; ++i) {
      var contentIndex = (i % contentString.length);
      bytes[i] = contentString.charCodeAt(contentIndex);
    }
    return result;
  }

  // See http://home.thep.lu.se/~bjorn/crc/ for more information on simple CRC32 calculations.
  function crc32ForByte(r) {
    for (let j = 0; j < 8; ++j) {
      r = (r & 1 ? 0 : 0xedb88320) ^ r >>> 1;
    }
    return r ^ 0xff000000;
  }

  function crc32(dataIterable) {
    const tableSize = 256;
    if (!window.crc32Table) {
      crc32Table = new Uint32Array(tableSize);
      for (let i = 0; i < tableSize; ++i) {
        crc32Table[i] = crc32ForByte(i);
      }
      window.crc32Table = crc32Table;
    }
    let dataBytes = new Uint8Array(dataIterable);
    let crc = 0;
    for (let i = 0; i < dataBytes.byteLength; ++i) {
      const crcLowByte = (crc & 0x000000ff);
      const dataByte = dataBytes[i];
      const tableIndex = crcLowByte ^ dataByte;
      // The last >>> is to convert this into an unsigned 32-bit integer.
      crc = (window.crc32Table[tableIndex] ^ (crc >>> 8)) >>> 0;
    }
    return crc;
  }

  // This is a small test function for the CRC32 implementation, not normally called but left in
  // for debugging purposes. We know the expected CRC32 of [97, 98, 99, 100, 101] is 2240272485,
  // or 0x8587d865, so if anything else is output we know there's an error in the implementation.
  function testCrc32() {
    const testArray = [97, 98, 99, 100, 101];
    const testArrayCrc32 = crc32(testArray);
    console.log('CRC32 for [97, 98, 99, 100, 101] is 0x' + testArrayCrc32.toString(16) + ' (' + testArrayCrc32 + ')');
  }

  async function sendFileBlock(fileContents, bytesAlreadySent) {
    let bytesRemaining = fileContents.byteLength - bytesAlreadySent;

    const maxBlockLength = 128;
    const blockLength = Math.min(bytesRemaining, maxBlockLength);
    var blockView = new Uint8Array(fileContents, bytesAlreadySent, blockLength);
    fileBlockCharacteristic.writeValue(blockView)
      .then(_ => {
        bytesRemaining -= blockLength;
        if ((bytesRemaining > 0) && isFileTransferInProgress) {
          msg('File block written - ' + bytesRemaining + ' bytes remaining');
          bytesAlreadySent += blockLength;
          sendFileBlock(fileContents, bytesAlreadySent);
        }
      })
      .catch(error => {
        console.log(error);
        msg('File block write error with ' + bytesRemaining + ' bytes remaining, see console');
      });
  }
</script>

</html>