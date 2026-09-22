(function () {
  const storageKey = "vortex-docs-theme";
  const root = document.documentElement;
  const systemTheme = window.matchMedia("(prefers-color-scheme: dark)");

  function savedTheme() {
    const value = window.localStorage.getItem(storageKey);
    return value === "light" || value === "dark" ? value : null;
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

  applyTheme(preferredTheme());

  document.addEventListener("DOMContentLoaded", function () {
    const searchArea = document.querySelector(".wy-side-nav-search");
    if (!searchArea || searchArea.querySelector(".theme-toggle")) {
      return;
    }

    const button = document.createElement("button");
    button.className = "theme-toggle";
    button.type = "button";
    button.addEventListener("click", function () {
      const nextTheme = root.dataset.theme === "dark" ? "light" : "dark";
      window.localStorage.setItem(storageKey, nextTheme);
      applyTheme(nextTheme);
    });
    searchArea.appendChild(button);
    applyTheme(preferredTheme());
  });

  systemTheme.addEventListener("change", function () {
    if (!savedTheme()) {
      applyTheme(preferredTheme());
    }
  });
})();
