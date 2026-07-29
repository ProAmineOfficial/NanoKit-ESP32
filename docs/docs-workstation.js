// Developed by Amine Saoud ibn al-Bashir.
import mermaid from "https://cdn.jsdelivr.net/npm/mermaid@11.4.1/dist/mermaid.esm.min.mjs";

const repositoryRaw = "https://raw.githubusercontent.com/ProAmineOfficial/NanoKit-ESP32/main";
const reducedMotion = window.matchMedia("(prefers-reduced-motion: reduce)").matches;
const finePointer = window.matchMedia("(hover: hover) and (pointer: fine)").matches;

window.lucide?.createIcons({ attrs: { "stroke-width": 1.7 } });

mermaid.initialize({
  startOnLoad: false,
  securityLevel: "strict",
  theme: "base",
  themeVariables: {
    background: "#091119",
    primaryColor: "#162633",
    primaryTextColor: "#e7f1f5",
    primaryBorderColor: "#8bcce4",
    lineColor: "#7bb8ce",
    secondaryColor: "#10202b",
    tertiaryColor: "#0d1821",
    clusterBkg: "#0e1b25",
    clusterBorder: "#486d7f",
    edgeLabelBackground: "#101c26",
    fontFamily: "Inter, system-ui, sans-serif"
  },
  flowchart: { htmlLabels: true, useMaxWidth: true }
});

function setUpNavigation() {
  const toggle = document.querySelector(".menu-toggle");
  const navigation = document.querySelector(".site-navigation");
  if (!toggle || !navigation) return;

  toggle.addEventListener("click", () => {
    const open = navigation.classList.toggle("is-open");
    toggle.setAttribute("aria-expanded", String(open));
    toggle.setAttribute("aria-label", open ? "Close navigation" : "Open navigation");
  });

  navigation.querySelectorAll("a").forEach((link) => link.addEventListener("click", () => {
    navigation.classList.remove("is-open");
    toggle.setAttribute("aria-expanded", "false");
    toggle.setAttribute("aria-label", "Open navigation");
  }));
}

function setUpReveals() {
  const targets = document.querySelectorAll("[data-reveal]");
  if (reducedMotion || !("IntersectionObserver" in window)) {
    targets.forEach((target) => target.classList.add("is-visible"));
    return;
  }

  const observer = new IntersectionObserver((entries) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      entry.target.classList.add("is-visible");
      observer.unobserve(entry.target);
    });
  }, { threshold: 0.12, rootMargin: "0px 0px -24px" });

  targets.forEach((target) => observer.observe(target));
}

function setUpPointerLighting() {
  if (!finePointer || reducedMotion) return;
  const interactive = document.querySelectorAll(".hero-product, .project-card, .category-card, .hardware-panel, .diagram-card, .graduation-section");

  interactive.forEach((element) => {
    let frame = 0;
    element.addEventListener("pointermove", (event) => {
      const bounds = element.getBoundingClientRect();
      const x = ((event.clientX - bounds.left) / bounds.width) * 100;
      const y = ((event.clientY - bounds.top) / bounds.height) * 100;
      const rotateY = ((x - 50) / 50) * 1.1;
      const rotateX = ((50 - y) / 50) * .8;
      cancelAnimationFrame(frame);
      frame = requestAnimationFrame(() => {
        element.style.setProperty("--pointer-x", `${x}%`);
        element.style.setProperty("--pointer-y", `${y}%`);
        if (element.classList.contains("hero-product")) {
          element.style.setProperty("--tilt-x", `${rotateX}deg`);
          element.style.setProperty("--tilt-y", `${rotateY}deg`);
        }
      });
    });
    element.addEventListener("pointerleave", () => {
      element.style.removeProperty("--tilt-x");
      element.style.removeProperty("--tilt-y");
    });
  });
}

function setUpTerminalTyping() {
  const terminals = [...document.querySelectorAll(".project-media[data-code]")];
  if (reducedMotion) {
    terminals.forEach((terminal) => { terminal.querySelector("code").textContent = terminal.dataset.code || ""; });
    return;
  }

  const startTyping = (terminal) => {
    if (terminal.dataset.typed === "true") return;
    terminal.dataset.typed = "true";
    const code = terminal.querySelector("code");
    const text = terminal.dataset.code || "";
    let index = 0;
    const type = () => {
      code.textContent = text.slice(0, index);
      index += 1;
      if (index <= text.length) window.setTimeout(type, 32);
    };
    type();
  };

  if (!("IntersectionObserver" in window)) {
    terminals.forEach(startTyping);
    return;
  }
  const observer = new IntersectionObserver((entries) => {
    entries.forEach((entry) => {
      if (!entry.isIntersecting) return;
      startTyping(entry.target);
      observer.unobserve(entry.target);
    });
  }, { threshold: .35 });
  terminals.forEach((terminal) => observer.observe(terminal));
}

async function renderRepositoryDiagram() {
  const destination = document.querySelector("#architecture-diagram");
  if (!destination) return;
  try {
    const response = await fetch(`${repositoryRaw}/README.md`);
    if (!response.ok) throw new Error("Unable to load the repository README.");
    const markdown = await response.text();
    const match = markdown.match(/```mermaid\s*([\s\S]*?)```/i);
    if (!match) throw new Error("No Mermaid diagram was found in the repository README.");
    const { svg, bindFunctions } = await mermaid.render("nanokit-repository-architecture", match[1].trim());
    destination.innerHTML = svg;
    bindFunctions?.(destination);
    destination.querySelectorAll(".node, .cluster, .edgePath path, .flowchart-link").forEach((element, index) => {
      element.style.setProperty("--diagram-index", index);
    });
  } catch (error) {
    destination.innerHTML = '<span class="diagram-loading">The source diagram remains available in the repository README.</span>';
    console.warn(error);
  }
}

setUpNavigation();
setUpReveals();
setUpPointerLighting();
setUpTerminalTyping();
renderRepositoryDiagram();
