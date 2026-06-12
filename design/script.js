Prism.languages.sleek = {
  // Single-line comments & multi-line comments
  comment: {
    pattern: /(\/\/.*|\/\*[\s\S]*?\*\/)/,
    greedy: true,
  },
  // Keywords
  keyword: {
    pattern:
    RegExp(`\\b(?:${[
      "fn", "let", "const", "var", "if", "else", "while", "for", "in", "return",
      "break", "continue", "match", "case", "default", "struct", "enum",
      "trait", "impl", "mod", "use", "pub", "as", "async", "await", "int", "float", "void", "bool", "string", "null", "char", "byte"
    ].join("|")})\\b`),
    greedy: true,
  },
  // Numbers
  number: {
    pattern: /\b(?:0x[0-9A-Fa-f]+|\d+)\b/,
    greedy: true,
  },
  // Double-quoted strings
  string: {
    pattern: /"[^"]*"/,
    greedy: true,
  },
  // Operators
  operator: {
    pattern: /[=!<>]+/,
    greedy: true,
  },
  function: {
    pattern: /\b\w+(?=\s*\()/,
    greedy: true,
  }
};




const markdownText = document.getElementById("markdown").innerHTML;
console.log(markdownText);
const article = document.createElement("article");
article.className = "markdown-body";
article.innerHTML = marked.parse(getRawHTMLFromString(markdownText.trim()));
document.body.appendChild(article);

function processMarkdown(main) {
  const newLines = main.querySelectorAll("wbr");
  for (const newLine of newLines) {
    newLine.outerHTML = "";
  }
  const children = main.querySelectorAll("md");
  console.log(children);
  for (const child of children) {
    const prev = child.innerHTML;
    const processed = marked.parse(getRawHTMLFromString(prev));
    console.log(prev, processed);
    child.outerHTML = processed;
  }
  const details = main.querySelectorAll("details");
  let count = 0;
  for (const detail of details) {
    count++;
    let hash = detail.querySelector("summary").textContent.trim().toLowerCase().replace(/\s+/g, '-');
    const is_closed = localStorage.getItem('details-closed-' + hash) === 'open' ? false : true;
    detail.dataset.hash = hash;
    if (is_closed) {
      detail.removeAttribute("open");
    } else {
      detail.setAttribute("open", "");
    }
    detail.addEventListener("toggle", () => {
      let hash = detail.dataset.hash;
      if (detail.open) {
        localStorage.setItem('details-closed-' + hash, 'open');
      } else {
        localStorage.setItem('details-closed-' + hash, 'closed');
      }
    });
  }
}

const highlight = () => {
  const markdown = document.querySelector(".markdown-body");
  if (markdown) {
    Prism.highlightAllUnder(markdown);
  }
  setTimeout(initCodeCopy, 200);
}

highlight();
setTimeout(() => {
  processMarkdown(article);
  highlight();
}, 100);

// Source - https://stackoverflow.com/a/73692596
// Posted by Daniil Loban, modified by community. See post 'Timeline' for change history
// Retrieved 2026-06-12, License - CC BY-SA 4.0

function initCodeCopy() {
  const codeBlocks = document.querySelectorAll('code[class*="language-"]');
  codeBlocks.forEach((block) => {
    const lang = parseLanguage(block);
    const referenceEl = block.parentElement;
    const parent = block.parentElement.parentElement;

    const wrapper = document.createElement("div");
    wrapper.className = "code-wrapper";
    parent.insertBefore(wrapper, referenceEl);
    wrapper.append(block.parentElement);

    const copyBtn = document.createElement("button");
    copyBtn.setAttribute("class", "copy-button");
    copyBtn.setAttribute("data-lang", lang);
    copyBtn.innerHTML = `${lang} <svg viewBox="0 0 24 24"><path fill="none" d="M0 0h24v24H0z"/><path d="M7 6V3a1 1 0 0 1 1-1h12a1 1 0 0 1 1 1v14a1 1 0 0 1-1 1h-3v3c0 .552-.45 1-1.007 1H4.007A1.001 1.001 0 0 1 3 21l.003-14c0-.552.45-1 1.007-1H7zM5.003 8L5 20h10V8H5.003zM9 6h8v10h2V4H9v2z" fill="currentColor"/></svg>`;

    wrapper.insertAdjacentElement("beforeend", copyBtn);
  });

  function parseLanguage(block) {
    const className = block.className;
    if (className.startsWith("language")) {
      const [prefix, lang] = className.split("-");
      return lang;
    }
  }

  async function fallbackCopyTextToClipboard(text) {
    return new Promise((resolve, reject) => {
      var textArea = document.createElement("textarea");
      textArea.value = copyInfo.getText();
      // Avoid scrolling to bottom
      textArea.style.top = "0";
      textArea.style.left = "0";
      textArea.style.position = "fixed";
      document.body.appendChild(textArea);
      textArea.focus();
      textArea.select();
      try {
        var successful = document.execCommand("copy");
        setTimeout(function () {
          if (successful) {
            resolve("success");
          } else {
            reject("error");
          }
        }, 1);
      } catch (err) {
        setTimeout(function () {
          reject(err);
        }, 1);
      } finally {
        document.body.removeChild(textArea);
      }
    });
  }

  async function copyTextToClipboard(text) {
    return new Promise((resolve, reject) => {
      if (navigator.clipboard) {
        navigator.clipboard.writeText(text).then(resolve(), function () {
          // try the fallback in case `writeText` didn't work
          fallbackCopyTextToClipboard(text).then(
            () => resolve(),
            () => reject(),
          );
        });
      } else {
        fallbackCopyTextToClipboard(text).then(
          () => resolve(),
          () => reject(),
        );
      }
    });
  }

  function copy(e) {
    const btn = e.currentTarget;
    const lang = btn.dataset.lang;
    const text = e.currentTarget.previousSibling.children[0].textContent;
    copyTextToClipboard(text).then(
      () => {
        btn.innerHTML = `copied! <svg viewBox="0 0 24 24"><path fill="none" d="M0 0h24v24H0z"/><path d="M7 6V3a1 1 0 0 1 1-1h12a1 1 0 0 1 1 1v14a1 1 0 0 1-1 1h-3v3c0 .552-.45 1-1.007 1H4.007A1.001 1.001 0 0 1 3 21l.003-14c0-.552.45-1 1.007-1H7zm2 0h8v10h2V4H9v2z" fill="currentColor"/></svg>`;
        btn.setAttribute("style", "opacity: 1");
      },
      () => alert("failed to copy"),
    );

    setTimeout(() => {
      btn.removeAttribute("style");
      btn.innerHTML = `${lang} <svg viewBox="0 0 24 24"><path fill="none" d="M0 0h24v24H0z"/><path d="M7 6V3a1 1 0 0 1 1-1h12a1 1 0 0 1 1 1v14a1 1 0 0 1-1 1h-3v3c0 .552-.45 1-1.007 1H4.007A1.001 1.001 0 0 1 3 21l.003-14c0-.552.45-1 1.007-1H7zM5.003 8L5 20h10V8H5.003zM9 6h8v10h2V4H9v2z" fill="currentColor"/></svg>`;
    }, 3000);
  }

  const copyButtons = document.querySelectorAll(".copy-button");

  copyButtons.forEach((btn) => {
    btn.addEventListener("click", copy);
  });
}
function getRawHTMLFromString(htmlString) {
  // Parse the HTML string into a document fragment
  const parser = new DOMParser();
  const doc = parser.parseFromString(htmlString, 'text/html');
  
  // Recursively walk the DOM and rebuild the HTML with decoded text
  function build(node) {
    if (node.nodeType === Node.TEXT_NODE) {
      // textContent gives the *decoded* characters (e.g., ">" instead of "&gt;")
      return node.textContent;
    }
    if (node.nodeType === Node.ELEMENT_NODE) {
      const tag = node.tagName.toLowerCase();
      let inner = '';
      for (const child of node.childNodes) {
        inner += build(child);
      }
      // Self-closing tags? Handle if needed (void elements)
      const voidTags = new Set(['br', 'hr', 'img', 'input', 'meta', 'link']);
      if (voidTags.has(tag)) {
        return `<${tag}>`;
      }
      return `<${tag}>${inner}</${tag}>`;
    }
    // Ignore comments, etc.
    return '';
  }
  
  let result = '';
  for (const child of doc.body.childNodes) {
    result += build(child);
  }
  return result;
}