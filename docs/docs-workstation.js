// Developed by Amine Saoud ibn al-Bashir.
import { marked } from "https://cdn.jsdelivr.net/npm/marked@12.0.2/lib/marked.esm.js";
import DOMPurify from "https://cdn.jsdelivr.net/npm/dompurify@3.1.6/dist/purify.es.mjs";
import mermaid from "https://cdn.jsdelivr.net/npm/mermaid@11.4.1/dist/mermaid.esm.min.mjs";

const REPOSITORY = "https://raw.githubusercontent.com/ProAmineOfficial/NanoKit-ESP32/main";
const SOURCE_REPOSITORY = "https://github.com/ProAmineOfficial/NanoKit-ESP32/blob/main";
const DEFAULT_DOCUMENT = "README.md";
const root = document.querySelector("#markdown-root");
const breadcrumb = document.querySelector("#breadcrumb");
const sourceLink = document.querySelector("#source-link");
const progressValue = document.querySelector("#reading-progress-value");
const navigationPanel = document.querySelector("#navigation-panel");
const navigationToggle = document.querySelector(".navigation-toggle");
const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
const finePointer = window.matchMedia("(hover: hover) and (pointer: fine)").matches;

let activePath = DEFAULT_DOCUMENT;

mermaid.initialize({
  startOnLoad: false,
  securityLevel: "strict",
  theme: "base",
  themeVariables: {
    background: "#0d1117",
    primaryColor: "#1b2731",
    primaryTextColor: "#e8eef5",
    primaryBorderColor: "#8eb9d0",
    lineColor: "#90aec0",
    secondaryColor: "#14212b",
    tertiaryColor: "#101820",
    clusterBkg: "#101820",
    clusterBorder: "#52758c",
    edgeLabelBackground: "#16222b",
    fontFamily: "Inter, system-ui, sans-serif"
  },
  flowchart: {
    htmlLabels: true,
    useMaxWidth: true
  }
});

function escapeHtml(value) {
  return value.replace(/[&<>"]/g, (character) => ({
    "&": "&amp;",
    "<": "&lt;",
    ">": "&gt;",
    '"': "&quot;"
  })[character]);
}

function cleanPath(value, fallback = DEFAULT_DOCUMENT) {
  if (!value) {
    return fallback;
  }

  const parts = [];
  for (const segment of value.replace(/\\/g, "/").split("/")) {
    if (!segment || segment === ".") {
      continue;
    }
    if (segment === "..") {
      parts.pop();
      continue;
    }
    parts.push(segment);
  }
  return parts.join("/") || fallback;
}

function currentDocumentPath() {
  return cleanPath(new URLSearchParams(window.location.search).get("path"));
}

function rawUrl(path) {
  return `${REPOSITORY}/${path.split("/").map(encodeURIComponent).join("/")}`;
}

function sourceUrl(path) {
  return `${SOURCE_REPOSITORY}/${path.split("/").map(encodeURIComponent).join("/")}`;
}

function splitHref(href) {
  const hashIndex = href.indexOf("#");
  return hashIndex === -1
    ? { path: href, hash: "" }
    : { path: href.slice(0, hashIndex), hash: href.slice(hashIndex) };
}

function resolveRepositoryPath(href) {
  const documentBase = new URL(`https://nanokit.local/${activePath}`);
  return cleanPath(new URL(href, documentBase).pathname.slice(1));
}

function mermaidCodeRenderer(token, legacyLanguage) {
  const code = typeof token === "object" ? token.text : token;
  const language = typeof token === "object" ? token.lang : legacyLanguage;
  if ((language || "").trim().toLowerCase() !== "mermaid") {
    return false;
  }
  return `<div class="diagram-panel" data-reveal><pre class="mermaid">${escapeHtml(code || "")}</pre></div>`;
}

const renderer = new marked.Renderer();
renderer.code = mermaidCodeRenderer;

function formatDocument(markdown) {
  return DOMPurify.sanitize(marked.parse(markdown, {
    gfm: true,
    breaks: false,
    renderer
  }));
}

function updateDocumentChrome() {
  breadcrumb.textContent = `NanoKit-ESP32 / ${activePath}`;
  sourceLink.href = sourceUrl(activePath);
  document.title = `${activePath.split("/").pop().replace(/\.md$/i, "")} | NanoKit-ESP32`;

  document.querySelectorAll("[data-doc-link]").forEach((link) => {
    const destination = new URL(link.href).searchParams.get("path");
    link.classList.toggle("is-active", cleanPath(destination) === activePath);
  });
}

function buildDocumentCards() {
  const contentNodes = Array.from(root.children);
  const fragment = document.createDocumentFragment();
  let currentCard = null;

  const createCard = (className = "doc-card") => {
    const card = document.createElement("section");
    card.className = className;
    card.dataset.reveal = "";
    fragment.append(card);
    return card;
  };

  for (const node of contentNodes) {
    if (node.matches("h1")) {
      const hero = createCard("document-hero");
      hero.append(node);
      currentCard = null;
      continue;
    }

    if (node.matches("h2")) {
      currentCard = createCard();
    }

    if (!currentCard) {
      currentCard = createCard();
    }
    currentCard.append(node);
  }

  root.replaceChildren(fragment);

  root.querySelectorAll("table").forEach((table) => {
    const scrollArea = document.createElement("div");
    scrollArea.className = "table-scroll";
    table.before(scrollArea);
    scrollArea.append(table);
  });
}

function rewriteLinksAndImages() {
  root.querySelectorAll("a[href]").forEach((link) => {
    const originalHref = link.getAttribute("href");
    if (!originalHref || originalHref.startsWith("#")) {
      return;
    }

    if (/^(https?:|mailto:|tel:)/i.test(originalHref)) {
      link.target = "_blank";
      link.rel = "noreferrer";
      return;
    }

    const { path, hash } = splitHref(originalHref);
    const resolved = resolveRepositoryPath(path);
    if (/\.md$/i.test(resolved)) {
      link.href = `?path=${encodeURIComponent(resolved)}${hash}`;
      link.dataset.documentPath = resolved;
      return;
    }
    link.href = `${rawUrl(resolved)}${hash}`;
    link.target = "_blank";
    link.rel = "noreferrer";
  });

  root.querySelectorAll("img[src]").forEach((image) => {
    const originalSource = image.getAttribute("src");
    if (originalSource && !/^(https?:|data:)/i.test(originalSource)) {
      image.src = rawUrl(resolveRepositoryPath(originalSource));
    }
  });
}

function animateMermaid() {
  root.querySelectorAll(".diagram-panel svg").forEach((svg) => {
    svg.setAttribute("role", "img");
    svg.querySelectorAll(".node, .cluster").forEach((element, index) => {
      element.style.setProperty("--diagram-index", index);
    });
    svg.querySelectorAll(".flowchart-link, .edgePath path, .edge-thickness-normal").forEach((element, index) => {
      element.style.setProperty("--diagram-index", index);
    });
  });
}

async function renderMermaid() {
  const diagrams = root.querySelectorAll(".mermaid");
  if (!diagrams.length) {
    return;
  }

  try {
    await mermaid.run({ nodes: diagrams });
    animateMermaid();
  } catch (error) {
    console.error("Mermaid rendering failed", error);
    root.querySelectorAll(".diagram-panel").forEach((panel) => panel.classList.add("diagram-error"));
  }
}

function setupReveal() {
  const targets = root.querySelectorAll("[data-reveal]");
  if (reducedMotion || !("IntersectionObserver" in window)) {
    targets.forEach((target) => target.classList.add("is-visible"));
    return;
  }

  const observer = new IntersectionObserver((entries, currentObserver) => {
    entries.forEach((entry) => {
      if (entry.isIntersecting) {
        entry.target.classList.add("is-visible");
        currentObserver.unobserve(entry.target);
      }
    });
  }, { threshold: 0.08, rootMargin: "0px 0px -24px" });

  targets.forEach((target) => observer.observe(target));
}

function setupPointerLight() {
  if (reducedMotion || !finePointer) {
    return;
  }

  root.querySelectorAll(".document-hero, .doc-card, .diagram-panel").forEach((surface) => {
    let frame = null;
    let latestEvent = null;

    surface.addEventListener("pointermove", (event) => {
      latestEvent = event;
      if (frame !== null) {
        return;
      }
      frame = requestAnimationFrame(() => {
        const bounds = surface.getBoundingClientRect();
        const x = ((latestEvent.clientX - bounds.left) / bounds.width) * 100;
        const y = ((latestEvent.clientY - bounds.top) / bounds.height) * 100;
        surface.style.setProperty("--pointer-x", `${x}%`);
        surface.style.setProperty("--pointer-y", `${y}%`);
        if (!surface.classList.contains("diagram-panel")) {
          surface.style.setProperty("--tilt-x", `${((y - 50) / -80).toFixed(2)}deg`);
          surface.style.setProperty("--tilt-y", `${((x - 50) / 80).toFixed(2)}deg`);
        }
        frame = null;
      });
    });

    surface.addEventListener("pointerleave", () => {
      surface.style.setProperty("--tilt-x", "0deg");
      surface.style.setProperty("--tilt-y", "0deg");
      surface.style.setProperty("--pointer-x", "50%");
      surface.style.setProperty("--pointer-y", "0%");
    });
  });
}

function updateReadingProgress() {
  const scrollableDistance = document.documentElement.scrollHeight - window.innerHeight;
  const progress = scrollableDistance > 0 ? (window.scrollY / scrollableDistance) * 100 : 0;
  progressValue.style.width = `${Math.min(100, Math.max(0, progress))}%`;
}

function showFailure(error) {
  root.innerHTML = `
    <section class="document-hero load-failure" data-reveal>
      <p class="eyebrow">DOCUMENT UNAVAILABLE</p>
      <h1>Unable to load ${escapeHtml(activePath)}</h1>
      <p>${escapeHtml(error.message || "The requested Markdown file could not be read.")}</p>
    </section>`;
  setupReveal();
}

async function loadDocument(path = currentDocumentPath()) {
  activePath = cleanPath(path);
  updateDocumentChrome();
  root.innerHTML = '<section class="document-hero" data-reveal><p class="eyebrow">NANOKIT ENGINEERING WORKSTATION</p><h1>Loading documentation</h1></section>';
  root.querySelector("[data-reveal]").classList.add("is-visible");

  try {
    const response = await fetch(rawUrl(activePath), { cache: "no-cache" });
    if (!response.ok) {
      throw new Error(`GitHub returned ${response.status}.`);
    }

    root.innerHTML = formatDocument(await response.text());
    rewriteLinksAndImages();
    buildDocumentCards();
    await renderMermaid();
    setupReveal();
    setupPointerLight();
    updateReadingProgress();
  } catch (error) {
    showFailure(error);
  }
}

document.addEventListener("click", (event) => {
  const documentLink = event.target.closest("a[data-document-path], a[data-doc-link]");
  if (!documentLink || event.metaKey || event.ctrlKey || event.shiftKey || event.altKey) {
    return;
  }

  const destination = documentLink.dataset.documentPath || new URL(documentLink.href).searchParams.get("path");
  if (!destination) {
    return;
  }

  event.preventDefault();
  const nextUrl = new URL(window.location.href);
  nextUrl.searchParams.set("path", cleanPath(destination));
  window.history.pushState({}, "", nextUrl);
  navigationPanel.classList.remove("is-open");
  navigationToggle.setAttribute("aria-expanded", "false");
  window.scrollTo({ top: 0, behavior: reducedMotion ? "auto" : "smooth" });
  loadDocument();
});

navigationToggle.addEventListener("click", () => {
  const open = navigationPanel.classList.toggle("is-open");
  navigationToggle.setAttribute("aria-expanded", String(open));
});

window.addEventListener("popstate", () => {
  window.scrollTo({ top: 0, behavior: "auto" });
  loadDocument();
});

window.addEventListener("scroll", updateReadingProgress, { passive: true });
document.body.classList.add("motion-ready");
loadDocument();
