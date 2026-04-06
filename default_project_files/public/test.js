function apply_changed_res(res) {
  if (res === "/test.js") {
    socket.close();
  }
  let name = window.location.origin + res;
  let scripts = Array.from(document.head.getElementsByTagName("script")).filter((e) => name.includes(e.src));
  let links = Array.from(document.head.getElementsByTagName("link")).filter((e) => name.includes(e.href));
  let element = scripts.concat(links)[0];
  if (element == undefined) return;
  const nextSibling = element.nextSibling;
  element.remove();
  if (nextSibling === null) {
    document.head.appendChild(element);
  } else {
    document.head.insertBefore(element, nextSibling)
  }
}

function apply_changed_pages(res) {
  if (res.includes(window.location.pathname)) {
    window.location.reload();
  }
}

function on_message(e) {
  const message = JSON.parse(e.data);
  if (message.changed_pages !== undefined) {
    apply_changed_pages(message.changed_pages);
  }
  if (message.changed_res !== undefined) {
    apply_changed_res(message.changed_res);
  }
  if (message.action !== undefined) {
    if (message.action === "close") {
      socket.close();
    }
    if (message.action === "refresh") {
      window.location.reload();
    }
  }
}

const socket = new WebSocket("ws://localhost:8081");

socket.onmessage = on_message;
