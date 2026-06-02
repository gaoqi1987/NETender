const fs = require('fs');
const path = require('path');

const storeFile = path.resolve('./localstorage.json');

function loadStore() {
  try {
    return JSON.parse(fs.readFileSync(storeFile, 'utf-8'));
  } catch (e) {
    return {};
  }
}

let store = loadStore();

function saveStore() {
  fs.writeFileSync(storeFile, JSON.stringify(store, null, 2));
}

if (typeof globalThis.localStorage === 'undefined') {
  globalThis.localStorage = {
    getItem(key) {
      return key in store ? store[key] : null;
    },
    setItem(key, value) {
      store[key] = String(value);
      saveStore();
    },
    removeItem(key) {
      delete store[key];
      saveStore();
    },
    clear() {
      store = {};
      saveStore();
    }
  };
}

// 定义 window 对象
globalThis.window = {
  isElectronNative: true,
  ipcRenderer: {
    send: () => {},
    sendSync: () => {},
    invoke: () => {},
    on: () => {},
    once: () => {},
  },
};

globalThis.navigator = {
  language: "zh-CN",
  userAgent:
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36",
};

const NERoom = require("./neroom-kit-node/dist/index.cjs.js");

const { NETenderWebSDK } = require("./netender-web-sdk/dist/index.cjs.js");

const roomkit = new NERoom();
roomkit._im = roomkit._im || {};
roomkit._im.nim = roomkit._im.nim || {};
roomkit._im.nim.logger = roomkit._im.nim.logger || console;

neTenderMeeting = NETenderWebSDK.getInstance({
  roomkit,
});

globalThis.neTenderMeeting = neTenderMeeting;
