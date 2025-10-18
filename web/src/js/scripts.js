let config = {}
let time = 0;
let version = 0;
let toastVisible = false;
let app;
var resetRequired = false;
var configInterval = null;

const hourhandstyles = ["simple", "wide", "split"]

function clamp(value, min, max) {
    return Math.min(Math.max(value, min), max);
}

function hexToHsv(hex) {
    if (!hex) {
        return null;
    }
    let normalized = hex.trim().toLowerCase();
    if (normalized.startsWith('#')) {
        normalized = normalized.slice(1);
    }
    if (normalized.length === 3) {
        normalized = normalized.split('').map(function (c) { return c + c; }).join('');
    }
    if (normalized.length !== 6 || /[^0-9a-f]/.test(normalized)) {
        return null;
    }

    const r = parseInt(normalized.slice(0, 2), 16) / 255;
    const g = parseInt(normalized.slice(2, 4), 16) / 255;
    const b = parseInt(normalized.slice(4, 6), 16) / 255;

    const max = Math.max(r, g, b);
    const min = Math.min(r, g, b);
    const delta = max - min;

    let hue = 0;
    if (delta !== 0) {
        if (max === r) {
            hue = ((g - b) / delta) % 6;
        } else if (max === g) {
            hue = (b - r) / delta + 2;
        } else {
            hue = (r - g) / delta + 4;
        }
        hue *= 60;
        if (hue < 0) {
            hue += 360;
        }
    }

    const saturation = max === 0 ? 0 : delta / max;
    const value = max;

    return { h: hue, s: saturation, v: value };
}

function hsvToHex(h, s, v) {
    const hue = ((h % 360) + 360) % 360;
    const saturation = clamp(s, 0, 1);
    const value = clamp(v, 0, 1);

    const chroma = value * saturation;
    const x = chroma * (1 - Math.abs(((hue / 60) % 2) - 1));
    const m = value - chroma;

    let r1 = 0;
    let g1 = 0;
    let b1 = 0;

    if (hue < 60) {
        r1 = chroma; g1 = x; b1 = 0;
    } else if (hue < 120) {
        r1 = x; g1 = chroma; b1 = 0;
    } else if (hue < 180) {
        r1 = 0; g1 = chroma; b1 = x;
    } else if (hue < 240) {
        r1 = 0; g1 = x; b1 = chroma;
    } else if (hue < 300) {
        r1 = x; g1 = 0; b1 = chroma;
    } else {
        r1 = chroma; g1 = 0; b1 = x;
    }

    const r = Math.round((r1 + m) * 255);
    const g = Math.round((g1 + m) * 255);
    const b = Math.round((b1 + m) * 255);

    return '#' + [r, g, b].map(function (channel) {
        const clamped = clamp(channel, 0, 255);
        return clamped.toString(16).padStart(2, '0');
    }).join('');
}

function initHueBrightnessControls() {
    function getBrightnessSliderSelector(colorId) {
        if (typeof CSS !== 'undefined' && CSS.escape) {
            return 'input.brightness-slider[data-color-input="' + CSS.escape(colorId) + '"]';
        }
        return 'input.brightness-slider[data-color-input="' + colorId.replace(/["\\]/g, '\\$&') + '"]';
    }

    function setupHueBrightnessGroups(root) {
        if (!root) {
            return;
        }

        const hueSliderCandidates = [];

        if (root instanceof Element && root.matches('input.hue-slider[data-color-input]')) {
            hueSliderCandidates.push(root);
        }

        if (root.querySelectorAll) {
            hueSliderCandidates.push.apply(hueSliderCandidates, Array.from(root.querySelectorAll('input.hue-slider[data-color-input]')));
        }

        hueSliderCandidates.forEach(function (hueSlider) {
            const colorId = hueSlider.dataset.colorInput;
            if (!colorId) {
                return;
            }

            const colorInput = document.getElementById(colorId);
            const brightnessSlider = document.querySelector(getBrightnessSliderSelector(colorId));

            initHueBrightnessGroup(colorInput, hueSlider, brightnessSlider);
        });
    }

    setupHueBrightnessGroups(document);

    if (typeof MutationObserver === 'undefined') {
        return;
    }

    const observer = new MutationObserver(function (mutations) {
        mutations.forEach(function (mutation) {
            mutation.addedNodes.forEach(function (node) {
                if (!(node instanceof Element)) {
                    return;
                }
                setupHueBrightnessGroups(node);
            });
        });
    });

    observer.observe(document.body, { childList: true, subtree: true });
}

function initHueBrightnessGroup(colorInput, hueSlider, brightnessSlider) {
    if (!colorInput || !hueSlider || !brightnessSlider) {
        return;
    }

    if (colorInput.dataset.hueBrightnessInit === 'true') {
        return;
    }

    colorInput.dataset.hueBrightnessInit = 'true';

    let saturation = 1;
    let isInternalUpdate = false;

    function setHueSliderTrack(hue) {
        const min = parseFloat(hueSlider.min) || 0;
        const max = parseFloat(hueSlider.max) || 360;
        const normalizedHue = ((hue % 360) + 360) % 360;
        const percentage = clamp((normalizedHue - min) / (max - min), 0, 1) * 100;
        const hueColor = hsvToHex(normalizedHue, 1, 1) || '#ff0000';
        const trackBackground = "linear-gradient(90deg, " + hueColor + " 0%, " + hueColor + " " + percentage + "%, #d8d8d8 " + percentage + "%, #d8d8d8 100%)";
        hueSlider.style.setProperty('--slider-track-bg', trackBackground);
    }

    function updateSliders(hexColor) {
        const hsv = hexToHsv(hexColor);
        if (!hsv) {
            return;
        }
        saturation = hsv.s;
        hueSlider.value = Math.round(hsv.h);
        brightnessSlider.value = Math.round(hsv.v * 100);
        setHueSliderTrack(hsv.h);
    }

    function updateColorFromSliders() {
        const hue = parseFloat(hueSlider.value) || 0;
        const brightness = clamp(parseFloat(brightnessSlider.value) / 100, 0, 1);
        const hexColor = hsvToHex(hue, saturation, brightness);

        setHueSliderTrack(hue);

        if (!hexColor || colorInput.value.toLowerCase() === hexColor) {
            return;
        }

        isInternalUpdate = true;
        colorInput.value = hexColor;
        colorInput.dispatchEvent(new Event('input', { bubbles: true }));
        colorInput.dispatchEvent(new Event('change', { bubbles: true }));
        isInternalUpdate = false;
    }

    function handleColorInputChange() {
        if (isInternalUpdate) {
            return;
        }
        updateSliders(colorInput.value);
    }

    hueSlider.addEventListener('input', updateColorFromSliders);
    brightnessSlider.addEventListener('input', updateColorFromSliders);

    colorInput.addEventListener('input', handleColorInputChange);
    colorInput.addEventListener('change', handleColorInputChange);

    updateSliders(colorInput.value);
    requestAnimationFrame(function () {
        updateSliders(colorInput.value);
    });
}

document.addEventListener("DOMContentLoaded", function () {

    initTabs();

    tinybind.formatters.sub = function (target, val) {
        return (target - val);
    };

    tinybind.formatters.power = function (ledCount, bgLedCount, bgOn) {
        if (!bgOn) {
            bgLedCount = 0;
        }
        let mAh = (ledCount + bgLedCount) * 50;
        return mAh;

    };

    tinybind.binders.barwidth = function (el, value) {
        let width = value / config.ledCount * 100;
        el.style.width = width + "%";
    }

    tinybind.binders.height = function (el, value) {
        if (value) {
            el.style.height = "2.666rem";
        } else {
            el.style.height = "0.666rem";
        }
    }

    tinybind.binders.barpos = function (el, value) {
        let margin = (value - 1) / config.ledCount * 100;
        el.style.marginLeft = margin + "%";
    };

    tinybind.formatters.time = {
        read: function (value) {
            let h = Math.floor(value / 60);
            h = String(h).padStart(2, '0');
            let m = value % 60;
            m = String(m).padStart(2, '0');
            return h + ":" + m;
        },
        publish: function (value) {
            const t = value.split(':');
            return (+t[0]) * 60 + (+t[1]);
        }
    }

    tinybind.formatters.replaceUnderscore = function (value) {
        return value.replace(/_/g, ' ');
    };

    tinybind.formatters.formatDate = function (value) {
        if (value != 0) {
            let date = new Date(value * 1000);
            return date.toLocaleString();
        } else {
            return "";
        }
    };
    tinybind.formatters.int = {
        read: function (value) {
            return value
        },
        publish: function (value) {
            return parseInt(value)
        }
    };
    tinybind.binders.baroverflow = function (el) {
        let indicator = document.getElementById("overflow-indicator");
        if (el.scrollWidth > el.clientWidth) {
            indicator.style.boxShadow = "-8px 0 4px -3px red inset";
        } else {
            indicator.style.boxShadow = "";
        }
    };

    getData('time').then(function (t) {
        time = t;
        getData('version').then(function (d) {
            version = d.toString();
            getConfig().then(function (c) {
                config = c;
            }).then(function () {
                app = tinybind.bind(document.getElementById("app"), {
                    config, version, time, timezones, languages, toggleFirmwareModal, toggleResetModal, toggleWifiModal, loadLanguage, toastVisible, hourhandstyles
                });
                initHueBrightnessControls();
                const bgLightToggle = document.getElementById("bglight");
                if (bgLightToggle) {
                    bgLightToggle.addEventListener("change", function () {
                        if (this.checked) {
                            window.setTimeout(initHueBrightnessControls, 0);
                        }
                    });
                }
            })
        })
    })

    initWatcher();
    let saveButton = document.getElementById("save-button");
    saveButton.addEventListener("click", function () {
        postConfig(true).then(c => { config = c });
    });

});

