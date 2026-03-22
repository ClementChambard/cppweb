function apply_changed_res(res) {
  let name = window.location.origin + res;
  let scripts = Array.from(document.head.getElementsByTagName("script")).filter((e) => name.includes(e.src));
  let links = Array.from(document.head.getElementsByTagName("link")).filter((e) => name.includes(e.href));
  let element = scripts.concat(links)[0];
  element.remove();
  document.head.appendChild(element);
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
}

const socket = new WebSocket("ws://localhost:8081");

socket.onmessage = on_message;
