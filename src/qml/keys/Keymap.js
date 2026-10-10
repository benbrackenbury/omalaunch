.pragma library

var CONTEXTS = ["list", "reader", "search", "settings", "page"]
var MAIL = ["list", "reader"]
var ANY = ["*"]

var BINDINGS = [
  { id: "cursorDown", keys: ["j", "Down"], contexts: MAIL,
    survivesOverlay: true,
    group: "Moving", label: "Move down",
    hintKey: "j / k", hint: { list: "move", reader: "next" } },
  { id: "cursorUp", keys: ["k", "Up"], contexts: MAIL,
    survivesOverlay: true,
    group: "Moving", label: "Move up" },
  { id: "scrollDown", keys: ["Shift+J"], contexts: ["reader"],
    survivesOverlay: true,
    group: "Moving", label: "Scroll down",
    hintKey: "Shift+J / Shift+K", hint: { reader: "scroll" } },
  { id: "scrollUp", keys: ["Shift+K"], contexts: ["reader"],
    survivesOverlay: true,
    group: "Moving", label: "Scroll up" },
  { id: "open", keys: ["Return", "Enter", "o", "l"], contexts: ["list"],
    group: "Moving", label: "Open the selected launch",
    hintKey: "o", hint: { list: "open" } },
  { id: "openReader", keys: ["Right"], contexts: ["list"],
    group: "Moving", label: "Open the selected launch" },
  { id: "backToList", keys: ["u", "h", "Left"], contexts: ["reader"],
    group: "Moving", label: "Back to the list",
    hintKey: "u", hint: { reader: "back" } },
  { id: "top", keys: ["g"], contexts: MAIL,
    group: "Moving", label: "Jump to the first launch" },
  { id: "bottom", keys: ["Shift+G"], contexts: MAIL,
    group: "Moving", label: "Jump to the last launch" },

  { id: "follow", keys: ["f"], contexts: MAIL,
    group: "Acting", label: "Follow or unfollow",
    hint: { list: "follow", reader: "follow" } },
  { id: "webcast", keys: ["w"], contexts: MAIL,
    group: "Acting", label: "Open the webcast" },
  { id: "spacex", keys: ["s"], contexts: MAIL,
    group: "Acting", label: "Open the SpaceX page" },

  { id: "search", keys: ["/"], contexts: MAIL.concat(["page"]),
    group: "Finding", label: "Search" },
  { id: "filter",
    keys: ["1", "2", "3", "4", "5"],
    contexts: ["list"], group: "Finding", label: "Filter missions",
    display: "1…5" },

  { id: "goPage",
    keys: ["Ctrl+1", "Ctrl+2", "Ctrl+3", "Ctrl+4", "Ctrl+5",
      "Ctrl+6", "Ctrl+7", "Ctrl+8", "Ctrl+9"],
    contexts: ANY, group: "Going", label: "Go to that sidebar page",
    display: "Ctrl+1…9",
    hintKey: "Ctrl", hint: { list: "pages", reader: "pages", page: "pages" } },
  { id: "settings", keys: ["Ctrl+,"], contexts: ANY,
    group: "Going", label: "Open settings" },

  { id: "refresh", keys: ["F5", "Ctrl+R", "r"], contexts: ANY,
    sequenceContexts: { "r": MAIL.concat(["page"]) },
    group: "App", label: "Refresh",
    hintKey: "r", hint: { list: "refresh", page: "refresh" } },
  { id: "help", keys: ["?", "Shift+/"], contexts: MAIL.concat(["page"]),
    survivesOverlay: true,
    group: "App", label: "Toggle all keybindings" },
  { id: "back", keys: ["Escape"], contexts: ANY,
    survivesOverlay: true,
    group: "App", label: "Back or leave the field",
    hint: { reader: "back", search: "leave", settings: "leave" } },
  { id: "quit", keys: ["Ctrl+Q"], contexts: ANY,
    group: "App", label: "Quit" }
]

function byId(id) {
  for (var i = 0; i < BINDINGS.length; i++) {
    if (BINDINGS[i].id === id) return BINDINGS[i]
  }
  return null
}

function slotFor(id, sequence) {
  var row = byId(id)
  var keys = row ? row.keys || [] : []
  return keys.indexOf(String(sequence || ""))
}

function matchesContext(binding, context) {
  if (!binding) return false
  var contexts = binding.contexts || []
  for (var i = 0; i < contexts.length; i++) {
    if (contexts[i] === "*" || contexts[i] === context) return true
  }
  return false
}

function matchesSequenceContext(binding, sequence, context) {
  if (!binding) return false
  var overrides = binding.sequenceContexts || ({})
  var contexts = overrides[String(sequence || "")] || binding.contexts || []
  for (var i = 0; i < contexts.length; i++) {
    if (contexts[i] === "*" || contexts[i] === context) return true
  }
  return false
}

function isSequenceEnabled(binding, sequence, context, overlay) {
  if (!matchesSequenceContext(binding, sequence, context)) return false
  if (overlay && !binding.survivesOverlay) return false
  return true
}

function bindingsFor(context) {
  var out = []
  for (var i = 0; i < BINDINGS.length; i++) {
    if (matchesContext(BINDINGS[i], context)) out.push(BINDINGS[i])
  }
  return out
}

function sequencesFor(context) {
  var out = []
  for (var i = 0; i < BINDINGS.length; i++) {
    var keys = BINDINGS[i].keys || []
    for (var k = 0; k < keys.length; k++) {
      if (matchesSequenceContext(BINDINGS[i], keys[k], context))
        out.push(({ id: BINDINGS[i].id, sequence: keys[k], binding: BINDINGS[i] }))
    }
  }
  return out
}

function readableSequence(sequence) {
  var text = String(sequence || "")
  text = text.replace("Return", "Enter")
  if (text === "Escape") return "Esc"
  if (text === "Shift+G") return "G"
  return text
}

function displayFor(binding) {
  if (!binding) return ""
  if (binding.display) return binding.display
  var keys = binding.keys || []
  var out = []
  for (var i = 0; i < keys.length; i++) {
    var readable = readableSequence(keys[i])
    if (out.indexOf(readable) < 0) out.push(readable)
  }
  return out.join(", ")
}

function hintKeyFor(binding) {
  if (!binding) return ""
  if (binding.hintKey) return binding.hintKey
  return displayFor(binding)
}

function keycapLabel(text) {
  return String(text || "").toLowerCase()
}

function hintTextFor(binding, context) {
  var hint = binding ? binding.hint : null
  if (!hint) return ""
  if (typeof hint === "string") return hint
  return hint[context] || ""
}

function helpGroups(hidden) {
  var groups = []
  var byName = ({})
  var omitted = Array.isArray(hidden) ? hidden : []
  for (var i = 0; i < BINDINGS.length; i++) {
    var binding = BINDINGS[i]
    if (omitted.indexOf(binding.id) >= 0) continue
    if (!byName[binding.group]) {
      byName[binding.group] = ({ name: binding.group, rows: [] })
      groups.push(byName[binding.group])
    }
    byName[binding.group].rows.push(({
      keys: displayFor(binding),
      action: binding.label
    }))
  }
  return groups
}

function hintsFor(context) {
  var out = []
  var rows = bindingsFor(context)
  for (var i = 0; i < rows.length; i++) {
    var text = hintTextFor(rows[i], context)
    if (text !== "") out.push(({ key: hintKeyFor(rows[i]), label: text }))
  }
  return out
}

function helpWeight(group) {
  return (group && Array.isArray(group.rows) ? group.rows.length : 0) + 1
}

function helpColumns(count, hidden) {
  var groups = helpGroups(hidden)
  var columns = Math.max(1, Math.min(groups.length, Math.floor(Number(count)) || 1))
  var out = []
  for (var c = 0; c < columns; c++) out.push([])
  if (columns === 1) {
    out[0] = groups
    return out
  }
  var total = 0
  for (var i = 0; i < groups.length; i++) total += helpWeight(groups[i])
  var at = 0
  var used = 0
  var placed = 0
  for (var g = 0; g < groups.length; g++) {
    var left = groups.length - g
    var free = columns - at
    var target = (total - placed) / free
    var nextWeight = helpWeight(groups[g])
    if (at < columns - 1 && out[at].length > 0
        && (left <= free - 1
          || Math.abs(used - target) <= Math.abs(used + nextWeight - target))) {
      placed += used
      at++
      used = 0
    }
    out[at].push(groups[g])
    used += helpWeight(groups[g])
  }
  return out
}
