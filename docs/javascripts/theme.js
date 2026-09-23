(function () {
  const storageKey = "vortex-docs-theme";
  const root = document.documentElement;
  const systemTheme = window.matchMedia("(prefers-color-scheme: dark)");

  // highlight.js runs highlightAll() on DOMContentLoaded, so registering here
  // (this script loads at the end of <body>) is early enough.
  if (window.hljs) {
    window.hljs.registerLanguage("vortex", function (hljs) {
      return {
        name: "Vortex",
        keywords: {
          keyword: "fn let mut struct return if else while for in break continue",
          type: "void bool char i32 u32 usize f32 f64 String",
          literal: "true false",
        },
        contains: [
          hljs.C_LINE_COMMENT_MODE,
          hljs.QUOTE_STRING_MODE,
          { scope: "string", begin: /'(\\.|[^\\'])'/ },
          hljs.C_NUMBER_MODE,
        ],
      };
    });
  }

  function savedTheme() {
    try {
      const value = window.localStorage.getItem(storageKey);
      return value === "light" || value === "dark" ? value : null;
    } catch (error) {
      return null;
    }
  }

  function preferredTheme() {
    return savedTheme() || (systemTheme.matches ? "dark" : "light");
  }

  function applyTheme(theme) {
    root.dataset.theme = theme;
    const button = document.querySelector(".theme-toggle");
    if (!button) {
      return;
    }
    const nextTheme = theme === "dark" ? "light" : "dark";
    button.textContent = `Use ${nextTheme} theme`;
    button.setAttribute("aria-label", `Use ${nextTheme} theme`);
    button.setAttribute("aria-pressed", theme === "dark" ? "true" : "false");
  }

  function addThemeToggle() {
    const searchArea = document.querySelector(".wy-side-nav-search");
    if (!searchArea || searchArea.querySelector(".theme-toggle")) {
      return;
    }

    const button = document.createElement("button");
    button.className = "theme-toggle";
    button.type = "button";
    button.addEventListener("click", function () {
      const nextTheme = root.dataset.theme === "dark" ? "light" : "dark";
      try {
        window.localStorage.setItem(storageKey, nextTheme);
      } catch (error) {
        // Storage can be unavailable; the toggle still works for this page.
      }
      applyTheme(nextTheme);
    });
    searchArea.appendChild(button);
    applyTheme(preferredTheme());
  }

  function addCopyButtons() {
    document.querySelectorAll(".rst-content pre").forEach(function (pre) {
      const code = pre.querySelector("code");
      if (!code || pre.querySelector(".copy-button")) {
        return;
      }
      const button = document.createElement("button");
      button.className = "copy-button";
      button.type = "button";
      button.textContent = "Copy";
      button.setAttribute("aria-label", "Copy code to clipboard");
      button.addEventListener("click", function () {
        navigator.clipboard.writeText(code.textContent).then(
          function () {
            button.textContent = "Copied";
          },
          function () {
            button.textContent = "Press Ctrl+C";
          }
        );
        window.setTimeout(function () {
          button.textContent = "Copy";
        }, 1600);
      });
      pre.appendChild(button);
    });
  }

  // Without JavaScript every .vx-step stays visible; with it, readers step
  // through one panel at a time.
  function enhanceSteppers() {
    document.querySelectorAll(".vx-stepper").forEach(function (stepper) {
      const steps = stepper.querySelectorAll(".vx-step");
      if (steps.length < 2 || stepper.classList.contains("is-enhanced")) {
        return;
      }
      let current = 0;
      const controls = document.createElement("div");
      controls.className = "vx-stepper-controls";
      const back = document.createElement("button");
      back.type = "button";
      back.textContent = "Back";
      const next = document.createElement("button");
      next.type = "button";
      next.textContent = "Next step";
      const status = document.createElement("span");
      status.className = "vx-stepper-status";
      status.setAttribute("aria-live", "polite");
      controls.append(back, next, status);
      stepper.appendChild(controls);
      stepper.classList.add("is-enhanced");

      function show(index) {
        current = index;
        steps.forEach(function (step, i) {
          step.classList.toggle("is-current", i === current);
        });
        back.disabled = current === 0;
        next.disabled = current === steps.length - 1;
        status.textContent = `Step ${current + 1} of ${steps.length}`;
      }

      back.addEventListener("click", function () {
        show(Math.max(0, current - 1));
      });
      next.addEventListener("click", function () {
        show(Math.min(steps.length - 1, current + 1));
      });
      show(0);
    });
  }

  // CSS animations are switched off by a prefers-reduced-motion rule in
  // extra.css; SVG <animate> elements need to be paused from script.
  function pauseSvgAnimationsIfReduced() {
    if (!window.matchMedia("(prefers-reduced-motion: reduce)").matches) {
      return;
    }
    document.querySelectorAll(".vx-figure svg").forEach(function (svg) {
      if (svg.pauseAnimations) {
        svg.pauseAnimations();
        svg.setCurrentTime(0);
      }
    });
  }

  applyTheme(preferredTheme());

  document.addEventListener("DOMContentLoaded", function () {
    addThemeToggle();
    addCopyButtons();
    enhanceSteppers();
    pauseSvgAnimationsIfReduced();
  });

  systemTheme.addEventListener("change", function () {
    if (!savedTheme()) {
      applyTheme(preferredTheme());
    }
  });
})();
