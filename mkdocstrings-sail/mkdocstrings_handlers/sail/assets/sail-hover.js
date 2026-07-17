/* IDE-style hover cards for mkdocstrings-sail.
 * Links rendered with data-sail-hover="<anchor>" reveal the matching
 * <template class="sail-hovercard" data-anchor="<anchor>"> as a fixed-
 * position card, viewport-aware, dismissed on mouse-out or scroll. */
(function () {
  "use strict";
  var card = null;

  var script = document.currentScript;
  var assetBase = script && script.src ? script.src.replace(/sail-hover\.js.*$/, "") : "assets/";

  function hide() {
    if (card) {
      card.remove();
      card = null;
    }
  }

  function position(target) {
    if (!card) return;
    var rect = target.getBoundingClientRect();
    var top = rect.bottom + 6;
    if (top + card.offsetHeight > window.innerHeight - 8) {
      top = Math.max(8, rect.top - card.offsetHeight - 6);
    }
    var left = Math.min(rect.left, window.innerWidth - card.offsetWidth - 12);
    card.style.top = top + "px";
    card.style.left = Math.max(8, left) + "px";
  }

  document.addEventListener("mouseover", function (event) {
    if (card && card.contains(event.target)) return; // allow hovering the card
    var target = event.target.closest("[data-sail-hover]");
    if (!target) {
      hide();
      return;
    }
    var key = target.getAttribute("data-sail-hover");
    if (card && card.dataset.for === key) return;
    hide();
    var isEip = key.indexOf("eip-") === 0;
    var isLean = key.indexOf("lean-") === 0;
    var template = document.querySelector(
      'template.sail-hovercard[data-anchor="' + (window.CSS ? CSS.escape(key) : key) + '"]'
    );
    if (!template && !isEip && !isLean) return;
    card = document.createElement("div");
    card.className = "sail-hovercard-float md-typeset";
    if (isEip) card.classList.add("sail-hovercard-eip");
    card.dataset.for = key;
    if (template) {
      card.appendChild(template.content.cloneNode(true));
    } else {
      // EIP cards have no baked content: the document is fetched and
      // rendered client-side on first view
      var doc = document.createElement("div");
      doc.className = "sail-hovercard-doc";
      doc.innerHTML = isEip ? "<p><em>Fetching " + key.toUpperCase() + "\u2026</em></p>" : "";
      card.appendChild(doc);
    }
    document.body.appendChild(card);
    position(target);
    if (isEip) liveEip(key.slice(4), target);
    if (isLean && !template) leanCard(key, target);
  });

  /* Lean definition cards are per-definition fragments generated at build
   * time (assets/lean-cards/<anchor>.html), fetched on first use so pages
   * embedding hundreds of references stay small. */
  var leanCache = {};
  function leanCard(key, target) {
    function render(html) {
      if (!card || card.dataset.for !== key) return;
      var doc = card.querySelector(".sail-hovercard-doc") || card;
      doc.outerHTML = html;
      position(target);
    }
    if (leanCache[key] === "pending" || leanCache[key] === "failed") return;
    if (leanCache[key]) {
      render(leanCache[key]);
      return;
    }
    leanCache[key] = "pending";
    fetch(assetBase + "lean-cards/" + key + ".html")
      .then(function (r) { if (!r.ok) throw new Error(r.status); return r.text(); })
      .then(function (html) {
        leanCache[key] = html;
        render(html);
      })
      .catch(function () { leanCache[key] = "failed"; });
  }

  /* Live EIP cards: the EIP's markdown source is fetched from the
   * canonical ethereum/EIPs repository on first use, rendered client-side
   * (vendored marked + highlight.js; untagged fences default to Python
   * per EIP-1 convention), and cached for a day. There is no build-time
   * EIP content. */
  var EIP_SOURCE = "https://raw.githubusercontent.com/ethereum/EIPs/master/EIPS/eip-";
  var EIP_TTL_MS = 24 * 60 * 60 * 1000;
  var eipCache = {};

  function cacheGet(n) {
    try {
      var raw = localStorage.getItem("sail-eip:" + n);
      if (!raw) return null;
      var entry = JSON.parse(raw);
      if (Date.now() - entry.t > EIP_TTL_MS) return null;
      return entry.h;
    } catch (e) {
      return null;
    }
  }

  function cachePut(n, html) {
    try {
      localStorage.setItem("sail-eip:" + n, JSON.stringify({ t: Date.now(), h: html }));
    } catch (e) {
      /* quota / private mode: memory cache still holds it */
    }
  }

  /* Remote markdown becomes innerHTML, so strip anything active. */
  function sanitize(root) {
    var bad = root.querySelectorAll("script, style, iframe, object, embed, link, meta");
    for (var i = 0; i < bad.length; i++) bad[i].remove();
    var all = root.querySelectorAll("*");
    for (var j = 0; j < all.length; j++) {
      var el = all[j];
      for (var k = el.attributes.length - 1; k >= 0; k--) {
        var a = el.attributes[k];
        if (/^on/i.test(a.name) || /^\s*javascript:/i.test(a.value)) el.removeAttribute(a.name);
      }
    }
  }

  function renderEipMarkdown(n, text) {
    if (!window.marked) return null;
    var title = "EIP-" + n;
    var m = text.match(/^---\r?\n([\s\S]*?)\r?\n---\r?\n/);
    if (m) {
      text = text.slice(m[0].length);
      var t = m[1].match(/^title:\s*(.+)$/m);
      if (t) title += ": " + t[1].trim().replace(/^["']|["']$/g, "");
    }
    var root = document.createElement("div");
    root.innerHTML = window.marked.parse(text, { gfm: true });
    sanitize(root);
    if (window.hljs) {
      var blocks = root.querySelectorAll("pre > code");
      for (var i = 0; i < blocks.length; i++) {
        var block = blocks[i];
        var lang = (block.className.match(/language-(\S+)/) || [])[1] || "python";
        if (window.hljs.getLanguage(lang)) {
          block.innerHTML = window.hljs.highlight(block.textContent, { language: lang }).value;
        }
      }
    }
    var heading = document.createElement("h1");
    heading.textContent = title;
    root.insertBefore(heading, root.firstChild);
    return root.innerHTML;
  }

  function liveEip(n, target) {
    function render(html) {
      if (!card || card.dataset.for !== "eip-" + n) return;
      var doc = card.querySelector(".sail-hovercard-doc") || card;
      doc.innerHTML = html;
    }
    if (eipCache[n] === "pending") return;
    if (eipCache[n] === "failed") {
      render("<p><em>EIP-" + n + " could not be fetched.</em></p>");
      return;
    }
    if (eipCache[n]) {
      render(eipCache[n]);
      return;
    }
    var cached = cacheGet(n);
    if (cached) {
      eipCache[n] = cached;
      render(cached);
      return;
    }
    eipCache[n] = "pending";
    fetch(EIP_SOURCE + n + ".md")
      .then(function (r) { if (!r.ok) throw new Error(r.status); return r.text(); })
      .then(function (text) {
        var html = renderEipMarkdown(n, text);
        if (!html) throw new Error("renderer unavailable");
        eipCache[n] = html;
        cachePut(n, html);
        render(html);
      })
      .catch(function () {
        eipCache[n] = "failed";
        render("<p><em>EIP-" + n + " could not be fetched.</em></p>");
      });
  }

  document.addEventListener(
    "scroll",
    function (event) {
      // scrolling inside the card keeps it open
      if (card && card.contains(event.target)) return;
      hide();
    },
    true
  );
})();
