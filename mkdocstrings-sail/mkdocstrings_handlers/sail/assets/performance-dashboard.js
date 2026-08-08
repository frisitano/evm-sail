(() => {
  "use strict";

  const OPTION_LIMIT = 200;
  const RANKING_LIMIT = 50;
  const PIPELINE = [
    ["Guest execution", null, 0],
    ["Stateless validation", "stateless_validation", 1],
    ["Decode input", "decode_input", 2],
    ["Index witness", "index_witness", 2],
    ["Index witness nodes", "index_witness_nodes", 3],
    ["Index witness codes", "index_witness_codes", 3],
    ["Index ancestor headers", "index_witness_headers", 3],
    ["Validate payload commitments", "validate_payload", 2],
    ["Execute block", "execute_block", 2],
    ["Block-start system calls", "block_start", 3],
    ["Transactions", "block_transactions", 3],
    ["Decode transactions", "tx_decode", 4],
    ["Reset transaction state", "tx_reset", 4],
    ["Validate transactions", "tx_validate", 4],
    ["Apply upfront costs", "tx_upfront", 4],
    ["Execute EVM frames", "tx_frame", 4],
    ["Settle transactions", "tx_settle", 4],
    ["Build receipts root", "receipts_root", 4],
    ["Finalize state", "block_end_state", 3],
    ["Build execution requests", "block_end_requests", 3],
    ["Validate result", "validate_result", 2],
    ["Build state root", "state_root", 3],
    ["Validate block access list", "block_access_list", 3],
    ["Compute output root", "compute_output_root", 1],
    ["Serialize output", "serialize_output", 1],
  ];
  const SCOPE_FAMILIES = [
    [/^htr_/, "SSZ hash-tree-root"],
    [/^(account|storage)_/, "State access"],
    [/^bal_/, "Block access list"],
    [/^tx_merge/, "Transaction merge"],
    [/^system_call_/, "System calls"],
    [/^request_/, "Execution requests"],
    [/^index_witness_/, "Witness indexing"],
  ];
  const PHASE_ORDER = [
    "input-decode",
    "execution",
    "state-root",
    "receipts-commitments",
  ];
  const PHASE_LABELS = {
    "input-decode": "Input decode",
    execution: "Execution",
    "state-root": "State root",
    "receipts-commitments": "Receipts & commitments",
  };
  const PHASE_CLASS_COUNT = 6;
  const number = new Intl.NumberFormat("en-US");
  const compact = new Intl.NumberFormat("en-US", {
    notation: "compact",
    maximumFractionDigits: 2,
  });

  function element(tag, className, text) {
    const node = document.createElement(tag);
    if (className) node.className = className;
    if (text !== undefined) node.textContent = text;
    return node;
  }

  function pretty(value) {
    return value
      .replace(/^for_/, "")
      .replaceAll("_", " ")
      .replace(/\b\w/g, (letter) => letter.toUpperCase());
  }

  function phaseLabel(name) {
    return PHASE_LABELS[name] || pretty(name.replaceAll("-", "_"));
  }

  function scopeFamily(name) {
    const match = SCOPE_FAMILIES.find(([pattern]) => pattern.test(name));
    return match ? match[1] : "Other";
  }

  function replaceOptions(select, values, selected, label, format = pretty) {
    select.replaceChildren();
    values.forEach((value) => {
      const option = element("option", null, format(value));
      option.value = value;
      select.append(option);
    });
    select.disabled = values.length === 0;
    select.setAttribute("aria-label", label);
    if (values.includes(selected)) select.value = selected;
  }

  function filteredOptions(items, query, label) {
    const needle = query.trim().toLocaleLowerCase();
    const matches = needle
      ? items.filter((item) => label(item).toLocaleLowerCase().includes(needle))
      : items;
    return { matches, visible: matches.slice(0, OPTION_LIMIT) };
  }

  function updateLimitNote(node, total, visible) {
    node.textContent =
      total > visible
        ? `Showing ${visible} of ${number.format(total)} matches. Type to narrow the list.`
        : `${number.format(total)} match${total === 1 ? "" : "es"}.`;
  }

  async function fetchJson(url) {
    const response = await fetch(url, { credentials: "same-origin" });
    if (!response.ok) {
      throw new Error(`${response.status} ${response.statusText}`);
    }
    return response.json();
  }

  function buildLabel(guest) {
    const build = guest.build;
    if (!build) return "build unknown";
    if (build.version) return build.version;
    if (build.commit) {
      const dirty = build.commit.endsWith("-dirty");
      const short = build.commit.replace(/-dirty$/, "").slice(0, 12);
      return dirty ? `${short} (dirty)` : short;
    }
    return "build unknown";
  }

  function buildTitle(guest) {
    const lines = [`Guest: ${guest.name}`];
    const build = guest.build;
    if (build?.commit) lines.push(`Built from commit ${build.commit}`);
    if (build?.version) lines.push(`Built from ${build.version}`);
    if (!build?.commit && !build?.version) {
      lines.push("Build provenance was not recorded for this run.");
    }
    if (guest.elf_sha256) lines.push(`ELF sha256 ${guest.elf_sha256}`);
    return lines.join("\n");
  }

  function guestNameCell(tag, guest) {
    const cell = element(tag, "evmsail-perf-guest-cell");
    cell.append(
      element("span", "evmsail-perf-guest-cell__name", guest.name),
      element("span", "evmsail-perf-build", buildLabel(guest)),
    );
    cell.title = buildTitle(guest);
    return cell;
  }

  function normalizeCase(profile, guests) {
    if (profile.guests) return profile;
    const guestName = guests[0]?.name;
    if (!guestName) return profile;
    return {
      id: profile.id,
      name: profile.name,
      block_index: profile.block_index,
      input_bytes: profile.input_bytes,
      gas_used: profile.gas_used,
      guests: {
        [guestName]: {
          total_steps: profile.total_steps,
          total_cost: profile.total_cost,
          scope_steps: profile.scope_steps,
          scope_costs: profile.scope_costs,
          opcode_costs: profile.opcode_costs || [],
          opcode_profile_completeness:
            profile.opcode_profile_completeness || "sdk_top_operations",
          top_cost_functions: profile.top_cost_functions || [],
          function_profile_status: profile.function_profile_status || "unknown",
          executed_functions: profile.executed_functions || [],
          function_inventory_status:
            profile.function_inventory_status || "unknown",
        },
      },
    };
  }

  function sumInto(target, source) {
    Object.entries(source || {}).forEach(([key, value]) => {
      if (Number.isFinite(value)) {
        target[key] = (target[key] || 0) + value;
      }
    });
  }

  function aggregateGuest(shard, guest) {
    const entries = shard.cases.map(
      (profile) => profile.guests?.[guest.name] || {},
    );
    const measured = entries.filter((entry) =>
      Number.isFinite(entry.total_steps),
    );
    const unavailable = entries.find((entry) => entry.unavailable)?.unavailable;
    const caseCount = shard.cases.length;
    const result = {
      ...guest,
      caseCount,
      measuredCases: measured.length,
      available: measured.length > 0,
      partial: measured.length > 0 && measured.length < caseCount,
      unavailableMessage: unavailable?.message || "Unavailable",
      totalSteps: null,
      totalCost: null,
      scopeSteps: {},
      scopeCosts: {},
      phases: null,
      opcodeCosts: [],
      executedFunctions: [],
      functionProfileStatus: "unknown",
    };
    if (!measured.length) return result;

    result.totalSteps = measured.reduce(
      (total, entry) => total + entry.total_steps,
      0,
    );
    result.totalCost = measured.every((entry) =>
      Number.isFinite(entry.total_cost),
    )
      ? measured.reduce((total, entry) => total + entry.total_cost, 0)
      : null;
    measured.forEach((entry) => {
      sumInto(result.scopeSteps, entry.scope_steps);
      sumInto(result.scopeCosts, entry.scope_costs);
    });
    result.functionProfileStatus = measured.some(
      (entry) => entry.function_profile_status === "stack_mismatch",
    )
      ? "stack_mismatch"
      : measured[0].function_profile_status || "unknown";

    if (measured.every((entry) => Array.isArray(entry.phases))) {
      const totals = new Map();
      measured.forEach((entry) => {
        entry.phases.forEach((phase) => {
          totals.set(phase.name, (totals.get(phase.name) || 0) + phase.steps);
        });
      });
      const names = [
        ...PHASE_ORDER.filter((name) => totals.has(name)),
        ...[...totals.keys()].filter((name) => !PHASE_ORDER.includes(name)),
      ];
      result.phases = names.map((name) => ({
        name,
        steps: totals.get(name),
      }));
    }

    const operations = new Map();
    measured.forEach((entry) => {
      (entry.opcode_costs || []).forEach((operation) => {
        const merged = operations.get(operation.name) || {
          name: operation.name,
          cost: 0,
          op_count: 0,
          frop_count: 0,
        };
        merged.cost += operation.cost || 0;
        merged.op_count += operation.op_count || 0;
        merged.frop_count += operation.frop_count || 0;
        operations.set(operation.name, merged);
      });
    });
    result.opcodeCosts = [...operations.values()]
      .map((operation) => ({
        ...operation,
        share_percent: result.totalCost
          ? (100 * operation.cost) / result.totalCost
          : 0,
      }))
      .sort((left, right) => right.cost - left.cost);

    const functions = new Map();
    measured.forEach((entry) => {
      (entry.executed_functions || []).forEach((fn) => {
        functions.set(
          fn.name,
          (functions.get(fn.name) || 0) + (fn.exclusive_steps || 0),
        );
      });
    });
    result.executedFunctions = [...functions.entries()]
      .map(([name, steps]) => ({
        name,
        exclusive_steps: steps,
        share_percent: result.totalSteps
          ? (100 * steps) / result.totalSteps
          : 0,
      }))
      .sort(
        (left, right) =>
          right.exclusive_steps - left.exclusive_steps ||
          left.name.localeCompare(right.name),
      );
    return result;
  }

  function aggregateShard(shard, guests) {
    const guestAggregates = guests.map((guest, index) =>
      Object.assign(aggregateGuest(shard, guest), { index }),
    );
    const gasValues = shard.cases
      .map((profile) => profile.gas_used)
      .filter((value) => Number.isFinite(value));
    return {
      caseCount: shard.cases.length,
      gasUsed: gasValues.length
        ? gasValues.reduce((total, value) => total + value, 0)
        : null,
      inputBytes: shard.cases.reduce(
        (total, profile) => total + (profile.input_bytes || 0),
        0,
      ),
      guests: guestAggregates,
      maxTotalSteps: Math.max(
        1,
        ...guestAggregates
          .map((guest) => guest.totalSteps)
          .filter((value) => Number.isFinite(value)),
      ),
    };
  }

  function bar(width, guestIndex) {
    const track = element("span", "evmsail-perf-comparison__track");
    const fill = element(
      "span",
      `evmsail-perf-comparison__bar evmsail-perf-guest-${guestIndex}`,
    );
    fill.style.width = `${Math.max(0, Math.min(100, width))}%`;
    track.append(fill);
    return track;
  }

  function comparisonCell(
    value,
    maximum,
    guestIndex,
    suffix = "",
    missingLabel = "Not instrumented",
  ) {
    const cell = element("td", "evmsail-perf-comparison");
    if (value === null || value === undefined) {
      cell.append(element("span", "evmsail-perf-unavailable", missingLabel));
      return cell;
    }
    cell.append(
      element(
        "span",
        "evmsail-perf-comparison__value",
        `${number.format(value)}${suffix}`,
      ),
      bar(maximum ? (value / maximum) * 100 : 0, guestIndex),
    );
    return cell;
  }

  function renderLegend(view, guests) {
    view.legend.replaceChildren(
      ...guests.map((guest, index) => {
        const item = element("span", "evmsail-perf-legend__item");
        item.title = buildTitle(guest);
        item.append(
          element(
            "span",
            `evmsail-perf-legend__swatch evmsail-perf-guest-${index}`,
          ),
          element("span", null, guest.name),
          element("span", "evmsail-perf-build", buildLabel(guest)),
        );
        return item;
      }),
    );
  }

  function fixtureRankLabel(fixture) {
    const category = fixture.category ? `${fixture.category}/` : "";
    return `${category}${fixture.fixture}`;
  }

  function renderRankings(view, catalog, guests, selectedId, onSelect) {
    const fixtures = catalog.fixtures;

    const stepsRanked = fixtures
      .map((fixture) => {
        const totals = guests.map(
          (guest) => fixture.guest_total_steps?.[guest.name] ?? null,
        );
        const finite = totals.filter((value) => Number.isFinite(value));
        return {
          fixture,
          totals,
          maxSteps: finite.length ? Math.max(...finite) : null,
        };
      })
      .filter((entry) => entry.maxSteps !== null)
      .sort((left, right) => right.maxSteps - left.maxSteps);
    const stepsShown = stepsRanked.slice(0, RANKING_LIMIT);
    const stepsScale = Math.max(
      1,
      ...stepsShown.map((entry) => entry.maxSteps),
    );

    const stepsFragment = document.createDocumentFragment();
    stepsShown.forEach((entry) => {
      const row = element("button", "evmsail-perf-rank-row");
      row.type = "button";
      if (entry.fixture.id === selectedId) {
        row.classList.add("evmsail-perf-rank-row--selected");
      }
      row.title =
        `${entry.fixture.path}\n` +
        guests
          .map((guest, index) =>
            entry.totals[index] === null
              ? `${guest.name}: unavailable`
              : `${guest.name}: ${number.format(entry.totals[index])} steps`,
          )
          .join("\n");
      row.addEventListener("click", () => onSelect(entry.fixture.id));
      const bars = element("span", "evmsail-perf-rank-row__bars");
      entry.totals.forEach((value, index) => {
        const track = element("span", "evmsail-perf-rank-row__track");
        if (value !== null) {
          const fill = element(
            "span",
            `evmsail-perf-rank-row__fill evmsail-perf-guest-${index}`,
          );
          fill.style.width = `${(value / stepsScale) * 100}%`;
          track.append(fill);
        }
        bars.append(track);
      });
      row.append(
        element(
          "span",
          "evmsail-perf-rank-row__label",
          fixtureRankLabel(entry.fixture),
        ),
        bars,
        element(
          "span",
          "evmsail-perf-rank-row__value",
          compact.format(entry.maxSteps),
        ),
      );
      stepsFragment.append(row);
    });
    view.stepsRanking.replaceChildren(stepsFragment);
    view.stepsRankingNote.textContent = stepsRanked.length
      ? `Top ${stepsShown.length} of ${number.format(stepsRanked.length)} ` +
        "fixtures, ranked by the largest guest step count. The row value is " +
        "that largest count."
      : "No fixture has complete step measurements for every embedded case.";

    const gasRanked = fixtures
      .map((fixture) => ({
        fixture,
        gas: Number.isFinite(fixture.total_gas_used)
          ? fixture.total_gas_used
          : Number.isFinite(fixture.max_gas_used)
            ? fixture.max_gas_used
            : null,
      }))
      .filter((entry) => entry.gas !== null)
      .sort((left, right) => right.gas - left.gas);
    const gasShown = gasRanked.slice(0, RANKING_LIMIT);
    const gasScale = Math.max(1, ...gasShown.map((entry) => entry.gas));

    const gasFragment = document.createDocumentFragment();
    gasShown.forEach((entry) => {
      const row = element("button", "evmsail-perf-rank-row");
      row.type = "button";
      if (entry.fixture.id === selectedId) {
        row.classList.add("evmsail-perf-rank-row--selected");
      }
      row.title = `${entry.fixture.path}\n${number.format(entry.gas)} gas used`;
      row.addEventListener("click", () => onSelect(entry.fixture.id));
      const bars = element("span", "evmsail-perf-rank-row__bars");
      const track = element("span", "evmsail-perf-rank-row__track");
      const fill = element(
        "span",
        "evmsail-perf-rank-row__fill evmsail-perf-rank-row__fill--gas",
      );
      fill.style.width = `${(entry.gas / gasScale) * 100}%`;
      track.append(fill);
      bars.append(track);
      row.append(
        element(
          "span",
          "evmsail-perf-rank-row__label",
          fixtureRankLabel(entry.fixture),
        ),
        bars,
        element(
          "span",
          "evmsail-perf-rank-row__value",
          compact.format(entry.gas),
        ),
      );
      gasFragment.append(row);
    });
    view.gasRanking.replaceChildren(gasFragment);
    view.gasRankingNote.textContent = gasRanked.length
      ? `Top ${gasShown.length} of ${number.format(gasRanked.length)} ` +
        "fixtures, ranked by block gas used."
      : "No fixture reports gas used.";
  }

  function renderSummary(view, aggregate) {
    const totals = aggregate.guests
      .filter((guest) => guest.available && !guest.partial)
      .map((guest) => guest.totalSteps);
    const best = totals.length ? Math.min(...totals) : null;
    const rankedGuests = [...aggregate.guests].sort((left, right) => {
      if (left.totalSteps === null && right.totalSteps === null) return 0;
      if (left.totalSteps === null) return 1;
      if (right.totalSteps === null) return -1;
      return right.totalSteps - left.totalSteps;
    });
    const body = document.createDocumentFragment();
    rankedGuests.forEach((guest) => {
      const row = document.createElement("tr");
      const nameCell = guestNameCell("td", guest);
      if (guest.partial) {
        nameCell.append(
          element(
            "span",
            "evmsail-perf-partial",
            `${guest.measuredCases}/${guest.caseCount} cases measured`,
          ),
        );
      }
      const comparable = guest.available && !guest.partial && best;
      row.append(
        nameCell,
        comparisonCell(
          guest.totalSteps,
          aggregate.maxTotalSteps,
          guest.index,
          "",
          guest.unavailableMessage,
        ),
        element(
          "td",
          "evmsail-perf-number",
          guest.totalCost === null ? "—" : number.format(guest.totalCost),
        ),
        element(
          "td",
          "evmsail-perf-number",
          comparable ? `${(guest.totalSteps / best).toFixed(2)}×` : "—",
        ),
        element(
          "td",
          "evmsail-perf-number",
          comparable ? `+${number.format(guest.totalSteps - best)}` : "—",
        ),
      );
      body.append(row);
    });
    view.summary.replaceChildren(body);
    view.summaryHelp.textContent =
      "Guests are ranked from most to fewest instruction steps for this " +
      "fixture. All bars use the largest guest step count as a shared " +
      "scale; relative and delta columns compare against the least " +
      "expensive fully measured guest.";
    const gasUsed = Number.isFinite(aggregate.gasUsed)
      ? ` · ${number.format(aggregate.gasUsed)} gas used`
      : "";
    const cases =
      aggregate.caseCount > 1
        ? ` · totals over ${aggregate.caseCount} embedded cases`
        : "";
    view.inputBytes.textContent =
      `${number.format(aggregate.inputBytes)} input bytes${gasUsed}${cases}`;
  }

  function renderPhases(view, aggregate) {
    const anyPhases = aggregate.guests.some((guest) => guest.phases);
    view.phasesSection.hidden = !anyPhases;
    if (!anyPhases) {
      view.phases.replaceChildren();
      view.phasesLegend.replaceChildren();
      return;
    }
    const phaseNames = [];
    aggregate.guests.forEach((guest) => {
      (guest.phases || []).forEach((phase) => {
        if (!phaseNames.includes(phase.name)) phaseNames.push(phase.name);
      });
    });
    const phaseClass = (name) => {
      const index = phaseNames.indexOf(name) % PHASE_CLASS_COUNT;
      return `evmsail-perf-phase-${index}`;
    };
    view.phasesLegend.replaceChildren(
      ...phaseNames.map((name) => {
        const item = element("span", "evmsail-perf-legend__item");
        item.append(
          element(
            "span",
            `evmsail-perf-legend__swatch ${phaseClass(name)}`,
          ),
          element("span", null, phaseLabel(name)),
        );
        return item;
      }),
      (() => {
        const item = element("span", "evmsail-perf-legend__item");
        item.append(
          element(
            "span",
            "evmsail-perf-legend__swatch evmsail-perf-phase-rest",
          ),
          element("span", null, "Unattributed"),
        );
        return item;
      })(),
    );

    const rankedGuests = [...aggregate.guests].sort((left, right) => {
      if (left.totalSteps === null && right.totalSteps === null) return 0;
      if (left.totalSteps === null) return 1;
      if (right.totalSteps === null) return -1;
      return right.totalSteps - left.totalSteps;
    });
    const fragment = document.createDocumentFragment();
    rankedGuests.forEach((guest) => {
      const row = element("div", "evmsail-perf-phase-row");
      const label = guestNameCell("span", guest);
      row.append(label);
      if (!guest.available) {
        row.append(
          element("span", "evmsail-perf-unavailable", guest.unavailableMessage),
        );
        fragment.append(row);
        return;
      }
      if (!guest.phases) {
        row.append(
          element(
            "span",
            "evmsail-perf-unavailable",
            "No phase data reported for this guest.",
          ),
        );
        fragment.append(row);
        return;
      }
      const track = element("span", "evmsail-perf-phase-row__track");
      let attributed = 0;
      guest.phases.forEach((phase) => {
        attributed += phase.steps;
        const segment = element(
          "span",
          `evmsail-perf-phase-row__segment ${phaseClass(phase.name)}`,
        );
        segment.style.width =
          `${(phase.steps / aggregate.maxTotalSteps) * 100}%`;
        segment.title =
          `${phaseLabel(phase.name)} — ${number.format(phase.steps)} steps` +
          ` (${((100 * phase.steps) / guest.totalSteps).toFixed(1)}%)`;
        track.append(segment);
      });
      const rest = guest.totalSteps - attributed;
      if (rest > 0) {
        const segment = element(
          "span",
          "evmsail-perf-phase-row__segment evmsail-perf-phase-rest",
        );
        segment.style.width = `${(rest / aggregate.maxTotalSteps) * 100}%`;
        segment.title =
          `Unattributed — ${number.format(rest)} steps` +
          ` (${((100 * rest) / guest.totalSteps).toFixed(1)}%)`;
        track.append(segment);
      }
      row.append(
        track,
        element(
          "span",
          "evmsail-perf-number",
          number.format(guest.totalSteps),
        ),
      );
      fragment.append(row);
    });
    view.phases.replaceChildren(fragment);
  }

  function renderProvenance(view, catalog, fixture, guests) {
    const generated = new Date(
      Number(catalog.generated_at_unix_ns) / 1_000_000,
    );
    const description =
      guests
        .map((guest) => `${guest.name} (${buildLabel(guest)})`)
        .join(" · ") +
      ` · ${fixture.path} · generated ` +
      generated.toISOString().slice(0, 10);
    view.provenance.replaceChildren(document.createTextNode(description));
    const sourceUrl = catalog.metadata?.source_url;
    if (typeof sourceUrl === "string" && /^https?:\/\//.test(sourceUrl)) {
      const source = element("a", "", "dataset source");
      source.href = sourceUrl;
      source.rel = "noreferrer";
      view.provenance.append(document.createTextNode(" · "), source);
    }
  }

  function renderPipeline(view, aggregate) {
    const fragment = document.createDocumentFragment();
    PIPELINE.forEach(([label, scope, depth]) => {
      const values = aggregate.guests.map((guest) =>
        !guest.available
          ? null
          : scope
            ? Object.hasOwn(guest.scopeSteps, scope)
              ? guest.scopeSteps[scope]
              : null
            : guest.totalSteps,
      );
      if (scope && values.every((value) => value === null)) return;
      const row = element("div", "evmsail-perf-scope");
      row.style.setProperty("--scope-depth", depth);
      row.append(element("div", "evmsail-perf-scope__name", label));
      const comparisons = element("div", "evmsail-perf-scope__comparisons");
      values.forEach((value, index) => {
        const guest = aggregate.guests[index];
        const item = element("div", "evmsail-perf-scope__guest");
        item.append(
          element("span", "evmsail-perf-scope__guest-name", guest.name),
          element(
            "span",
            value === null
              ? "evmsail-perf-unavailable"
              : "evmsail-perf-number",
            value === null
              ? guest.available
                ? "Not instrumented"
                : guest.unavailableMessage
              : number.format(value),
          ),
        );
        if (value !== null) {
          item.append(bar((value / aggregate.maxTotalSteps) * 100, index));
        }
        comparisons.append(item);
      });
      row.append(comparisons);
      fragment.append(row);
    });
    view.pipeline.replaceChildren(fragment);
    view.pipelineHelp.textContent =
      "Indentation shows logical containment. Every guest bar uses the same " +
      "absolute instruction-step scale. Scope totals are inclusive; missing " +
      "tags are shown as not instrumented, never as zero.";
  }

  function renderDetails(view, aggregate) {
    const scopeNames = new Set();
    aggregate.guests.forEach((guest) => {
      Object.keys(guest.scopeSteps).forEach((name) => scopeNames.add(name));
    });
    const rows = [...scopeNames]
      .map((name) => ({
        name,
        values: aggregate.guests.map((guest) =>
          Object.hasOwn(guest.scopeSteps, name)
            ? guest.scopeSteps[name]
            : null,
        ),
      }))
      .sort(
        (left, right) =>
          Math.max(...right.values.filter((value) => value !== null)) -
          Math.max(...left.values.filter((value) => value !== null)),
      );
    const body = document.createDocumentFragment();
    rows.forEach(({ name, values }) => {
      const row = document.createElement("tr");
      const maximum = Math.max(1, ...values.filter((value) => value !== null));
      row.append(element("td", null, pretty(name)));
      row.append(element("td", null, scopeFamily(name)));
      values.forEach((value, index) => {
        const guest = aggregate.guests[index];
        row.append(
          comparisonCell(
            value,
            maximum,
            index,
            "",
            guest.available ? "Not instrumented" : guest.unavailableMessage,
          ),
        );
      });
      body.append(row);
    });
    view.details.replaceChildren(body);
    view.detailsEmpty.hidden = rows.length !== 0;
    view.detailsHeader.replaceChildren(
      element("th", null, "Scope"),
      element("th", null, "Family"),
      ...aggregate.guests.map((guest) => element("th", null, guest.name)),
    );
  }

  function renderOperations(view, aggregate) {
    const byName = new Map();
    aggregate.guests.forEach((guest, guestIndex) => {
      guest.opcodeCosts.forEach((entry) => {
        if (!byName.has(entry.name)) {
          byName.set(
            entry.name,
            Array(aggregate.guests.length).fill(null),
          );
        }
        byName.get(entry.name)[guestIndex] = entry;
      });
    });
    const rows = [...byName.entries()].sort(
      (left, right) =>
        Math.max(...right[1].map((entry) => entry?.cost || 0)) -
        Math.max(...left[1].map((entry) => entry?.cost || 0)),
    );
    const body = document.createDocumentFragment();
    rows.forEach(([name, entries]) => {
      const row = document.createElement("tr");
      const maximum = Math.max(1, ...entries.map((entry) => entry?.cost || 0));
      row.append(element("td", "evmsail-perf-function", name));
      entries.forEach((entry, index) => {
        if (!entry) {
          const guest = aggregate.guests[index];
          row.append(
            comparisonCell(
              null,
              maximum,
              index,
              "",
              guest.available ? "Not used" : guest.unavailableMessage,
            ),
          );
          return;
        }
        const cell = comparisonCell(
          entry.cost,
          maximum,
          index,
          ` · ${entry.share_percent.toFixed(2)}%`,
        );
        const executions =
          (entry.op_count || 0) + (entry.frop_count || 0);
        cell.insertBefore(
          element(
            "span",
            "evmsail-perf-comparison__meta",
            `${number.format(executions)} executions · ` +
              `OP ${number.format(entry.op_count || 0)} · ` +
              `FROP ${number.format(entry.frop_count || 0)}`,
          ),
          cell.lastChild,
        );
        row.append(cell);
      });
      body.append(row);
    });
    view.operations.replaceChildren(body);
    view.operationsEmpty.hidden = rows.length !== 0;
    view.operationsHeader.replaceChildren(
      element("th", null, "ZisK operation"),
      ...aggregate.guests.map((guest) => element("th", null, guest.name)),
    );
  }

  function renderFunctions(view, aggregate) {
    const needle = view.functionSearch.value.trim().toLocaleLowerCase();
    const fragment = document.createDocumentFragment();
    aggregate.guests.forEach((guest) => {
      const block = element("div", "evmsail-perf-function-block");
      const heading = guestNameCell("h3", guest);
      heading.classList.add("evmsail-perf-function-block__title");
      block.append(heading);
      if (!guest.available) {
        block.append(
          element("p", "evmsail-perf-empty", guest.unavailableMessage),
        );
        fragment.append(block);
        return;
      }
      if (guest.functionProfileStatus === "stack_mismatch") {
        block.append(
          element(
            "p",
            "evmsail-perf-status evmsail-perf-status--warning",
            "ZisK could not maintain an inclusive call stack for this " +
              "guest. The table remains complete because it assigns each " +
              "executed instruction to its containing ELF symbol instead.",
          ),
        );
      }
      const allFunctions = guest.executedFunctions;
      const rows = needle
        ? allFunctions.filter((entry) =>
            entry.name.toLocaleLowerCase().includes(needle),
          )
        : allFunctions;
      const visible = rows.slice(0, OPTION_LIMIT);
      const maximum = Math.max(
        1,
        ...allFunctions.map((entry) => entry.exclusive_steps || 0),
      );
      if (!allFunctions.length) {
        block.append(
          element(
            "p",
            "evmsail-perf-empty",
            `No executed function inventory was reported for ${guest.name}.`,
          ),
        );
        fragment.append(block);
        return;
      }
      const wrap = element("div", "evmsail-perf-table-wrap");
      const table = document.createElement("table");
      const head = document.createElement("thead");
      const headerRow = document.createElement("tr");
      headerRow.append(
        element("th", null, "Exact ELF symbol"),
        element("th", null, "Exclusive executed steps"),
        element("th", null, "Share of guest steps"),
      );
      head.append(headerRow);
      const body = document.createElement("tbody");
      visible.forEach((entry) => {
        const row = document.createElement("tr");
        const nameCell = element("td", "evmsail-perf-function", entry.name);
        nameCell.title = entry.name;
        row.append(
          nameCell,
          comparisonCell(entry.exclusive_steps, maximum, guest.index),
          element(
            "td",
            "evmsail-perf-number",
            `${entry.share_percent.toFixed(2)}%`,
          ),
        );
        body.append(row);
      });
      table.append(head, body);
      wrap.append(table);
      const note = element("small", "evmsail-perf-limit-note");
      updateLimitNote(note, rows.length, visible.length);
      block.append(note, wrap);
      if (!rows.length) {
        block.append(
          element(
            "p",
            "evmsail-perf-empty",
            "No executed function symbols match this filter.",
          ),
        );
      }
      fragment.append(block);
    });
    view.functions.replaceChildren(fragment);
  }

  function createView(root) {
    const controls = element("div", "evmsail-perf-controls");

    const fixtureSearch = document.createElement("input");
    fixtureSearch.type = "search";
    fixtureSearch.placeholder = "Filter fixture paths";
    const fixtureSelect = document.createElement("select");
    const fixtureNote = element("small", "evmsail-perf-limit-note");
    const fixtureField = element(
      "label",
      "evmsail-perf-field evmsail-perf-field--wide",
    );
    fixtureField.append(
      element("span", null, "Fixture"),
      fixtureSearch,
      fixtureSelect,
      fixtureNote,
    );
    controls.append(fixtureField);

    const status = element(
      "div",
      "evmsail-perf-status",
      "Loading ZisK profile catalog…",
    );
    status.setAttribute("role", "status");
    const provenance = element("p", "evmsail-perf-provenance");
    const legend = element("div", "evmsail-perf-legend");
    const inputBytes = element("span", "evmsail-perf-input");

    const rankingSection = element("section", "evmsail-perf-section");
    const rankingGrid = element("div", "evmsail-perf-rankings");
    const stepsRankingPanel = element("div", "evmsail-perf-ranking");
    const stepsRankingNote = element("p", "evmsail-perf-help");
    const stepsRanking = element("div", "evmsail-perf-ranking__rows");
    stepsRankingPanel.append(
      element("h3", null, "By instruction steps"),
      stepsRankingNote,
      stepsRanking,
    );
    const gasRankingPanel = element("div", "evmsail-perf-ranking");
    const gasRankingNote = element("p", "evmsail-perf-help");
    const gasRanking = element("div", "evmsail-perf-ranking__rows");
    gasRankingPanel.append(
      element("h3", null, "By gas used"),
      gasRankingNote,
      gasRanking,
    );
    rankingGrid.append(stepsRankingPanel, gasRankingPanel);
    rankingSection.append(
      element("h2", null, "Fixture rankings"),
      element(
        "p",
        "evmsail-perf-help",
        "Both rankings are sorted from high to low. Select any row to load " +
          "that fixture's full comparison below.",
      ),
      rankingGrid,
    );

    const summarySection = element("section", "evmsail-perf-section");
    const summaryHelp = element("p", "evmsail-perf-help");
    const summaryWrap = element("div", "evmsail-perf-table-wrap");
    const summaryTable = document.createElement("table");
    const summaryHead = document.createElement("thead");
    const summaryHeader = document.createElement("tr");
    summaryHeader.append(
      element("th", null, "Guest"),
      element("th", null, "Instruction steps"),
      element("th", null, "Proving cost"),
      element("th", null, "Relative to best"),
      element("th", null, "Delta from best"),
    );
    summaryHead.append(summaryHeader);
    const summary = document.createElement("tbody");
    summaryTable.append(summaryHead, summary);
    summaryWrap.append(summaryTable);
    summarySection.append(
      element("h2", null, "Guest comparison"),
      summaryHelp,
      inputBytes,
      summaryWrap,
    );

    const phasesSection = element("section", "evmsail-perf-section");
    phasesSection.hidden = true;
    const phasesLegend = element("div", "evmsail-perf-legend");
    const phases = element("div", "evmsail-perf-phases");
    phasesSection.append(
      element("h2", null, "Per-phase step breakdown"),
      element(
        "p",
        "evmsail-perf-help",
        "Each guest bar is split into the coarse validation phases " +
          "reported by its instrumentation: input decode, execution, state " +
          "root, and receipts/commitments. Segment widths share the same " +
          "absolute step scale as the comparison above; the muted tail is " +
          "unattributed guest overhead. Guests without phase data are " +
          "labelled, never shown as zero.",
      ),
      phasesLegend,
      phases,
    );

    const pipelineSection = element("section", "evmsail-perf-section");
    const pipelineHelp = element("p", "evmsail-perf-help");
    const pipeline = element("div", "evmsail-perf-pipeline");
    pipelineSection.append(
      element("h2", null, "Inclusive pipeline scope diagram"),
      pipelineHelp,
      pipeline,
    );

    const detailSection = element("section", "evmsail-perf-section");
    const detailWrap = element("div", "evmsail-perf-table-wrap");
    const detailTable = document.createElement("table");
    const detailHead = document.createElement("thead");
    const detailsHeader = document.createElement("tr");
    detailHead.append(detailsHeader);
    const details = document.createElement("tbody");
    detailTable.append(detailHead, details);
    detailWrap.append(detailTable);
    const detailsEmpty = element(
      "p",
      "evmsail-perf-empty",
      "No semantic scopes were reported for this fixture.",
    );
    detailSection.append(
      element("h2", null, "All reported semantic scopes"),
      element(
        "p",
        "evmsail-perf-help",
        "This includes pipeline, cross-cutting, and implementation scopes. Each row has a shared scale across guests; tags are comparable only when guests instrument the same semantic boundary.",
      ),
      detailWrap,
      detailsEmpty,
    );

    const operationSection = element("section", "evmsail-perf-section");
    const operationWrap = element("div", "evmsail-perf-table-wrap");
    const operationTable = document.createElement("table");
    const operationHead = document.createElement("thead");
    const operationsHeader = document.createElement("tr");
    operationHead.append(operationsHeader);
    const operations = document.createElement("tbody");
    operationTable.append(operationHead, operations);
    operationWrap.append(operationTable);
    const operationsEmpty = element(
      "p",
      "evmsail-perf-empty",
      "No ZisK operation-cost rows were reported for these guests.",
    );
    operationSection.append(
      element("h2", null, "All used ZisK operation costs"),
      element(
        "p",
        "evmsail-perf-help",
        "Every costed operation used by at least one guest is shown. Cost combines ordinary OP and FROP execution. “Not used” means zero occurrences for this input, not a truncated report. These are ZisK operations, not EVM opcodes.",
      ),
      operationWrap,
      operationsEmpty,
    );

    const functionSection = element("section", "evmsail-perf-section");
    const functionControls = element(
      "div",
      "evmsail-perf-function-controls",
    );
    const functionSearchField = element("label", "evmsail-perf-field");
    functionSearchField.append(element("span", null, "Filter symbols"));
    const functionSearch = document.createElement("input");
    functionSearch.type = "search";
    functionSearch.placeholder = "Function or symbol name";
    functionSearchField.append(functionSearch);
    functionControls.append(functionSearchField);
    const functions = element("div");
    functionSection.append(
      element("h2", null, "Executed functions"),
      element(
        "p",
        "evmsail-perf-help",
        "Every guest lists the ELF symbols that executed at least one ZisK instruction. Steps are exclusive to each symbol's address range, so the rows partition that guest's total instruction steps.",
      ),
      functionControls,
      functions,
    );

    root.replaceChildren(
      controls,
      status,
      provenance,
      legend,
      rankingSection,
      summarySection,
      phasesSection,
      operationSection,
      pipelineSection,
      detailSection,
      functionSection,
    );
    return {
      fixtureSearch,
      fixtureSelect,
      fixtureNote,
      status,
      provenance,
      legend,
      inputBytes,
      stepsRanking,
      stepsRankingNote,
      gasRanking,
      gasRankingNote,
      summary,
      summaryHelp,
      phasesSection,
      phasesLegend,
      phases,
      pipeline,
      pipelineHelp,
      details,
      detailsEmpty,
      detailsHeader,
      operations,
      operationsEmpty,
      operationsHeader,
      functionSearch,
      functions,
    };
  }

  async function initializeRoot(root) {
    if (root.dataset.initialized === "true") return;
    root.dataset.initialized = "true";
    const view = createView(root);
    let catalog;
    try {
      catalog = await fetchJson(new URL(root.dataset.catalog, document.baseURI));
    } catch (error) {
      view.status.classList.add("evmsail-perf-status--error");
      view.status.textContent =
        "No generated profile dataset is available in this documentation build. " +
        "Generate it with tools/benchmark_zisk.py --profile sdk --dashboard-dir … " +
        `(${error.message}).`;
      return;
    }

    const guests = catalog.guests || (catalog.guest ? [catalog.guest] : []);
    const fixtures = catalog.fixtures;
    let fixtureMatches = [];
    let shard = null;
    let pendingFixtureId = null;
    renderLegend(view, guests);

    function fixtureLabel(fixture) {
      const category = fixture.category ? `${fixture.category}/` : "";
      const gas = Number.isFinite(fixture.total_gas_used)
        ? ` · ${number.format(fixture.total_gas_used)} gas`
        : Number.isFinite(fixture.max_gas_used)
          ? ` · max ${number.format(fixture.max_gas_used)} gas`
          : "";
      return (
        `${category}${fixture.fixture} (${number.format(fixture.case_count)})` +
        gas
      );
    }

    function orderedByGas(items) {
      return [...items].sort((left, right) => {
        const leftGas = Number.isFinite(left.total_gas_used)
          ? left.total_gas_used
          : Number.isFinite(left.max_gas_used)
            ? left.max_gas_used
            : null;
        const rightGas = Number.isFinite(right.total_gas_used)
          ? right.total_gas_used
          : Number.isFinite(right.max_gas_used)
            ? right.max_gas_used
            : null;
        if (leftGas === null && rightGas === null) return 0;
        if (leftGas === null) return 1;
        if (rightGas === null) return -1;
        return (
          rightGas - leftGas ||
          fixtureLabel(left).localeCompare(fixtureLabel(right))
        );
      });
    }

    function rebuildFixtures() {
      const grouped = orderedByGas(fixtures);
      const result = filteredOptions(
        grouped,
        view.fixtureSearch.value,
        fixtureLabel,
      );
      fixtureMatches = result.matches;
      const previous = pendingFixtureId || view.fixtureSelect.value;
      pendingFixtureId = null;
      replaceOptions(
        view.fixtureSelect,
        result.visible.map((fixture) => fixture.id),
        result.visible.some((fixture) => fixture.id === previous)
          ? previous
          : result.visible[0]?.id,
        "Fixture",
        (id) => fixtureLabel(grouped.find((fixture) => fixture.id === id)),
      );
      updateLimitNote(
        view.fixtureNote,
        result.matches.length,
        result.visible.length,
      );
      loadFixture();
    }

    function selectFixture(id) {
      pendingFixtureId = id;
      view.fixtureSearch.value = "";
      rebuildFixtures();
    }

    async function loadFixture() {
      const fixture = fixtureMatches.find(
        (candidate) => candidate.id === view.fixtureSelect.value,
      );
      renderRankings(view, catalog, guests, fixture?.id, selectFixture);
      if (!fixture) {
        view.status.textContent = "No fixture matches the current filter.";
        return;
      }
      view.status.textContent = `Loading ${fixture.path}…`;
      try {
        shard = await fetchJson(
          new URL(fixture.shard, new URL(root.dataset.catalog, document.baseURI)),
        );
        shard.cases = shard.cases.map((profile) =>
          normalizeCase(profile, guests),
        );
      } catch (error) {
        view.status.classList.add("evmsail-perf-status--error");
        view.status.textContent = `Could not load fixture profile: ${error.message}`;
        return;
      }
      view.status.classList.remove("evmsail-perf-status--error");
      renderSelectedFixture(fixture);
    }

    function renderSelectedFixture(fixture) {
      const aggregate = aggregateShard(shard, guests);
      const unavailable = aggregate.guests.filter(
        (guest) => !guest.available,
      ).length;
      view.status.textContent =
        `${number.format(catalog.fixture_count)} fixture files · ` +
        `${number.format(catalog.case_count)} profiled cases · ` +
        `${guests.length} guests` +
        (unavailable
          ? ` · ${unavailable} unavailable measurement${unavailable === 1 ? "" : "s"}`
          : "");
      renderProvenance(view, catalog, fixture, guests);
      renderSummary(view, aggregate);
      renderPhases(view, aggregate);
      renderPipeline(view, aggregate);
      renderDetails(view, aggregate);
      renderOperations(view, aggregate);
      renderFunctions(view, aggregate);
    }

    view.fixtureSearch.addEventListener("input", rebuildFixtures);
    view.fixtureSelect.addEventListener("change", loadFixture);
    view.functionSearch.addEventListener("input", () => {
      const fixture = fixtureMatches.find(
        (candidate) => candidate.id === view.fixtureSelect.value,
      );
      if (fixture && shard) renderSelectedFixture(fixture);
    });

    rebuildFixtures();
  }

  function initialize() {
    document
      .querySelectorAll("[data-evmsail-performance-dashboard]")
      .forEach(initializeRoot);
  }

  if (typeof document$ !== "undefined") {
    document$.subscribe(initialize);
  } else if (document.readyState === "loading") {
    document.addEventListener("DOMContentLoaded", initialize, { once: true });
  } else {
    initialize();
  }
})();
