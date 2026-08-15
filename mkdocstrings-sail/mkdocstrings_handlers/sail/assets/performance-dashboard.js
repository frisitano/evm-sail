(() => {
  "use strict";

  const OPTION_LIMIT = 200;
  const RANKING_LIMIT = 50;
  const SCOPE_FAMILIES = [
    [/^htr_/, "SSZ hash-tree-root"],
    [/^(account|storage)_/, "State access"],
    [/^bal_/, "Block access list"],
    [/^tx_merge/, "Transaction merge"],
    [/^system_call_/, "System calls"],
    [/^request_/, "Execution requests"],
    [/^index_witness_/, "Witness indexing"],
  ];
  const PHASE_SCOPES = [
    ["input-decode", "decode_input"],
    ["witness-indexing", "index_witness"],
    ["execution", "execute_block"],
    ["state-root", "state_root"],
    ["receipts-commitments", "receipts_root"],
  ];
  const PHASE_ORDER = PHASE_SCOPES.map(([phase]) => phase);
  const PHASE_SCOPE_TAGS = new Map(PHASE_SCOPES);
  const PHASE_LABELS = {
    "input-decode": "Input decode",
    "witness-indexing": "Witness indexing",
    execution: "Execution",
    "state-root": "State root",
    "receipts-commitments": "Receipts & commitments",
  };
  const PHASE_CLASS_COUNT = 6;
  const MEASURES = {
    steps: {
      id: "steps",
      label: "Instruction steps",
      lower: "instruction steps",
      noun: "steps",
      note:
        "Every view below reports ZisK instruction steps: the deterministic " +
        "execution size of the guest run.",
      guestTotal: (guest) => guest.totalSteps,
      guestScopes: (guest) => guest.scopeSteps,
      phaseValue: (phase) => phase.steps,
      aggregateMax: (aggregate) => aggregate.maxTotalSteps,
      fixtureTotals: (fixture) => fixture.guest_total_steps,
    },
    cost: {
      id: "cost",
      label: "Proving cost",
      lower: "proving cost",
      noun: "cost",
      note:
        "Every view below reports ZisK proving cost: the weighted cost the " +
        "prover charges for the same run.",
      guestTotal: (guest) => guest.totalCost,
      guestScopes: (guest) => guest.scopeCosts,
      phaseValue: (phase) => phase.cost,
      aggregateMax: (aggregate) => aggregate.maxTotalCost,
      fixtureTotals: (fixture) => fixture.guest_total_cost,
    },
  };
  const MEASURE_ORDER = ["steps", "cost"];
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

  function otherMeasure(measure) {
    return MEASURES[measure.id === "steps" ? "cost" : "steps"];
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
          phases: profile.phases,
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

  function aggregatePhases(measured) {
    if (!measured.every((entry) => Array.isArray(entry.phases))) return null;
    const steps = new Map();
    measured.forEach((entry) => {
      entry.phases.forEach((phase) => {
        steps.set(phase.name, (steps.get(phase.name) || 0) + phase.steps);
      });
    });
    const names = [
      ...PHASE_ORDER.filter((name) => steps.has(name)),
      ...[...steps.keys()].filter((name) => !PHASE_ORDER.includes(name)),
    ];
    return names.map((name) => {
      const tag = PHASE_SCOPE_TAGS.get(name);
      const costs = tag
        ? measured.map((entry) => entry.scope_costs?.[tag])
        : [];
      const cost =
        tag && costs.length && costs.every((value) => Number.isFinite(value))
          ? costs.reduce((total, value) => total + value, 0)
          : null;
      return { name, steps: steps.get(name), cost };
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
    result.phases = aggregatePhases(measured);

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
    const finite = (pick) =>
      guestAggregates.map(pick).filter((value) => Number.isFinite(value));
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
      maxTotalSteps: Math.max(1, ...finite((guest) => guest.totalSteps)),
      maxTotalCost: Math.max(1, ...finite((guest) => guest.totalCost)),
    };
  }

  function rankGuests(guests, measure) {
    return [...guests].sort((left, right) => {
      const leftValue = measure.guestTotal(left);
      const rightValue = measure.guestTotal(right);
      if (!Number.isFinite(leftValue) && !Number.isFinite(rightValue)) return 0;
      if (!Number.isFinite(leftValue)) return 1;
      if (!Number.isFinite(rightValue)) return -1;
      return rightValue - leftValue;
    });
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
    if (!Number.isFinite(value)) {
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

  function measureHeaderCell(guest, measure) {
    const cell = element("th");
    cell.append(
      element("span", null, guest.name),
      element("span", "evmsail-perf-th-measure", measure.label),
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

  function fixtureGas(fixture) {
    if (Number.isFinite(fixture.total_gas_used)) {
      return { value: fixture.total_gas_used, exact: true };
    }
    if (Number.isFinite(fixture.max_gas_used)) {
      return { value: fixture.max_gas_used, exact: false };
    }
    return null;
  }

  function gasLabel(gas, formatter) {
    return gas.exact
      ? `${formatter.format(gas.value)} gas`
      : `max ${formatter.format(gas.value)} gas`;
  }

  function renderRankings(view, catalog, guests, measure, selectedId, onSelect) {
    const fixtures = catalog.fixtures;

    view.measureRankingTitle.textContent = `By ${measure.lower}`;
    const measureRanked = fixtures
      .map((fixture) => {
        const totals = guests.map((guest) => {
          const value = measure.fixtureTotals(fixture)?.[guest.name];
          return Number.isFinite(value) ? value : null;
        });
        const finite = totals.filter((value) => value !== null);
        return {
          fixture,
          totals,
          gas: fixtureGas(fixture),
          maxValue: finite.length ? Math.max(...finite) : null,
        };
      })
      .filter((entry) => entry.maxValue !== null)
      .sort((left, right) => right.maxValue - left.maxValue);
    const measureShown = measureRanked.slice(0, RANKING_LIMIT);
    const measureScale = Math.max(
      1,
      ...measureShown.map((entry) => entry.maxValue),
    );

    const measureFragment = document.createDocumentFragment();
    measureShown.forEach((entry) => {
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
              : `${guest.name}: ${number.format(entry.totals[index])} ` +
                `${measure.noun}`,
          )
          .join("\n") +
        (entry.gas ? `\n${gasLabel(entry.gas, number)}` : "");
      row.addEventListener("click", () => onSelect(entry.fixture.id));
      const bars = element("span", "evmsail-perf-rank-row__bars");
      entry.totals.forEach((value, index) => {
        const track = element("span", "evmsail-perf-rank-row__track");
        if (value !== null) {
          const fill = element(
            "span",
            `evmsail-perf-rank-row__fill evmsail-perf-guest-${index}`,
          );
          fill.style.width = `${(value / measureScale) * 100}%`;
          track.append(fill);
        }
        bars.append(track);
      });
      const label = element("span", "evmsail-perf-rank-row__label");
      label.append(
        element(
          "span",
          "evmsail-perf-rank-row__name",
          fixtureRankLabel(entry.fixture),
        ),
      );
      if (entry.gas) {
        label.append(
          element(
            "span",
            "evmsail-perf-rank-row__meta",
            gasLabel(entry.gas, compact),
          ),
        );
      }
      row.append(
        label,
        bars,
        element(
          "span",
          "evmsail-perf-rank-row__value",
          compact.format(entry.maxValue),
        ),
      );
      measureFragment.append(row);
    });
    view.measureRanking.replaceChildren(measureFragment);
    view.measureRankingNote.textContent = measureRanked.length
      ? `Top ${measureShown.length} of ${number.format(measureRanked.length)} ` +
        `fixtures, ranked by the largest guest ${measure.lower} total. The ` +
        "row value is that largest total."
      : `No fixture reports ${measure.lower} for every embedded case.`;
  }

  function renderSummary(view, aggregate, measure) {
    const secondary = otherMeasure(measure);
    const maximum = measure.aggregateMax(aggregate);
    const totals = aggregate.guests
      .filter((guest) => guest.available && !guest.partial)
      .map((guest) => measure.guestTotal(guest))
      .filter((value) => Number.isFinite(value));
    const best = totals.length ? Math.min(...totals) : null;
    const body = document.createDocumentFragment();
    rankGuests(aggregate.guests, measure).forEach((guest) => {
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
      const value = measure.guestTotal(guest);
      const comparable =
        guest.available && !guest.partial && best && Number.isFinite(value);
      const secondaryValue = secondary.guestTotal(guest);
      row.append(
        nameCell,
        comparisonCell(
          value,
          maximum,
          guest.index,
          "",
          guest.available ? "Not instrumented" : guest.unavailableMessage,
        ),
        element(
          "td",
          "evmsail-perf-number",
          Number.isFinite(secondaryValue)
            ? number.format(secondaryValue)
            : "—",
        ),
        element(
          "td",
          "evmsail-perf-number",
          comparable ? `${(value / best).toFixed(2)}×` : "—",
        ),
        element(
          "td",
          "evmsail-perf-number",
          comparable ? `+${number.format(value - best)}` : "—",
        ),
      );
      body.append(row);
    });
    view.summary.replaceChildren(body);
    view.summaryPrimaryHeader.textContent = measure.label;
    view.summarySecondaryHeader.textContent = secondary.label;
    view.summaryHelp.textContent =
      `Guests are ranked from most to fewest ${measure.lower} for this ` +
      `fixture. All bars use the largest guest ${measure.lower} total as a ` +
      "shared scale; relative and delta columns compare against the least " +
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

  function renderPhases(view, aggregate, measure) {
    const anyPhases = aggregate.guests.some((guest) => guest.phases);
    view.phasesSection.hidden = !anyPhases;
    view.phasesTitle.textContent = `Per-phase ${measure.lower} breakdown`;
    view.phasesHelp.textContent =
      "Each guest bar is split into the coarse validation phases reported by " +
      "its instrumentation: input decode, witness indexing, execution, " +
      "state root, and receipts/commitments. Segment widths share the same " +
      `absolute ${measure.lower} scale as the comparison above; the muted ` +
      "tail is unattributed guest overhead. Guests without phase data are " +
      "labelled, never shown as zero.";
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

    const maximum = measure.aggregateMax(aggregate);
    const fragment = document.createDocumentFragment();
    rankGuests(aggregate.guests, measure).forEach((guest) => {
      const row = element("div", "evmsail-perf-phase-row");
      row.append(guestNameCell("span", guest));
      if (!guest.available) {
        row.append(
          element("span", "evmsail-perf-unavailable", guest.unavailableMessage),
        );
        fragment.append(row);
        return;
      }
      const total = measure.guestTotal(guest);
      const values = (guest.phases || []).map((phase) => ({
        name: phase.name,
        value: measure.phaseValue(phase),
      }));
      const attributable =
        values.length &&
        Number.isFinite(total) &&
        values.every((phase) => Number.isFinite(phase.value));
      if (!attributable) {
        row.append(
          element(
            "span",
            "evmsail-perf-unavailable",
            `No phase ${measure.lower} data reported for this guest.`,
          ),
        );
        fragment.append(row);
        return;
      }
      const track = element("span", "evmsail-perf-phase-row__track");
      let attributed = 0;
      values.forEach((phase) => {
        attributed += phase.value;
        const segment = element(
          "span",
          `evmsail-perf-phase-row__segment ${phaseClass(phase.name)}`,
        );
        segment.style.width = `${(phase.value / maximum) * 100}%`;
        segment.title =
          `${phaseLabel(phase.name)} — ${number.format(phase.value)} ` +
          `${measure.noun} (${((100 * phase.value) / total).toFixed(1)}%)`;
        track.append(segment);
      });
      const rest = total - attributed;
      if (rest > 0) {
        const segment = element(
          "span",
          "evmsail-perf-phase-row__segment evmsail-perf-phase-rest",
        );
        segment.style.width = `${(rest / maximum) * 100}%`;
        segment.title =
          `Unattributed — ${number.format(rest)} ${measure.noun}` +
          ` (${((100 * rest) / total).toFixed(1)}%)`;
        track.append(segment);
      }
      row.append(
        track,
        element("span", "evmsail-perf-number", number.format(total)),
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

  function renderDetails(view, aggregate, measure) {
    const scopeNames = new Set();
    aggregate.guests.forEach((guest) => {
      Object.keys(measure.guestScopes(guest)).forEach((name) =>
        scopeNames.add(name),
      );
    });
    const rows = [...scopeNames]
      .map((name) => ({
        name,
        values: aggregate.guests.map((guest) => {
          const scopes = measure.guestScopes(guest);
          return Object.hasOwn(scopes, name) ? scopes[name] : null;
        }),
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
    view.detailsHelp.textContent =
      "This includes pipeline, cross-cutting, and implementation scopes, " +
      `reported as ${measure.lower}. Each row has a shared scale across ` +
      "guests; tags are comparable only when guests instrument the same " +
      "semantic boundary.";
    view.detailsHeader.replaceChildren(
      element("th", null, "Scope"),
      element("th", null, "Family"),
      ...aggregate.guests.map((guest) => measureHeaderCell(guest, measure)),
    );
  }

  function renderOperations(view, aggregate) {
    const cost = MEASURES.cost;
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
        // Non-breaking spaces keep each counter with its label, so a narrow
        // guest column folds this line only at the separators.
        cell.insertBefore(
          element(
            "span",
            "evmsail-perf-comparison__meta",
            `${number.format(executions)}\u00a0executions · ` +
              `OP\u00a0${number.format(entry.op_count || 0)} · ` +
              `FROP\u00a0${number.format(entry.frop_count || 0)}`,
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
      ...aggregate.guests.map((guest) => measureHeaderCell(guest, cost)),
    );
  }

  function renderFunctions(view, aggregate, measure, selectedName, onSelect) {
    const guests = aggregate.guests;
    const active =
      guests.find((guest) => guest.name === selectedName) || guests[0];

    view.functionTabs.replaceChildren(
      ...guests.map((guest, position) => {
        const selected = guest === active;
        const tab = element("button", "evmsail-perf-tab");
        tab.type = "button";
        tab.setAttribute("role", "tab");
        tab.setAttribute("aria-selected", selected ? "true" : "false");
        tab.setAttribute("aria-controls", view.functionPanel.id);
        tab.tabIndex = selected ? 0 : -1;
        tab.title = buildTitle(guest);
        tab.append(
          element(
            "span",
            `evmsail-perf-tab__swatch evmsail-perf-guest-${guest.index}`,
          ),
          element("span", "evmsail-perf-tab__name", guest.name),
          element("span", "evmsail-perf-build", buildLabel(guest)),
        );
        tab.addEventListener("click", () => onSelect(guest.name));
        tab.addEventListener("keydown", (event) => {
          const step =
            event.key === "ArrowRight"
              ? 1
              : event.key === "ArrowLeft"
                ? -1
                : null;
          if (step !== null) {
            event.preventDefault();
            const next = (position + step + guests.length) % guests.length;
            onSelect(guests[next].name);
          } else if (event.key === "Home") {
            event.preventDefault();
            onSelect(guests[0].name);
          } else if (event.key === "End") {
            event.preventDefault();
            onSelect(guests[guests.length - 1].name);
          }
        });
        return tab;
      }),
    );

    view.functionPanel.setAttribute(
      "aria-label",
      active ? `Executed functions for ${active.name}` : "Executed functions",
    );
    view.functionMeasureNote.hidden = measure.id === "steps";
    view.functionMeasureNote.textContent =
      measure.id === "steps"
        ? ""
        : "ZisK attributes proving cost per operation, not per ELF symbol, " +
          "so this inventory stays in exclusive instruction steps.";

    if (!active || !active.available) {
      view.functions.replaceChildren();
      view.functionTable.hidden = true;
      view.functionsStatus.hidden = true;
      view.functionNote.textContent = "";
      view.functionsEmpty.hidden = false;
      view.functionsEmpty.textContent = active
        ? active.unavailableMessage
        : "No guest is available for this fixture.";
      return;
    }

    const stackMismatch = active.functionProfileStatus === "stack_mismatch";
    view.functionsStatus.hidden = !stackMismatch;
    view.functionsStatus.textContent = stackMismatch
      ? "ZisK could not maintain an inclusive call stack for this guest. " +
        "The table remains complete because it assigns each executed " +
        "instruction to its containing ELF symbol instead."
      : "";

    const allFunctions = active.executedFunctions;
    const needle = view.functionSearch.value.trim().toLocaleLowerCase();
    const matches = needle
      ? allFunctions.filter((entry) =>
          entry.name.toLocaleLowerCase().includes(needle),
        )
      : allFunctions;
    const visible = matches.slice(0, OPTION_LIMIT);
    const maximum = Math.max(
      1,
      ...allFunctions.map((entry) => entry.exclusive_steps || 0),
    );
    const body = document.createDocumentFragment();
    visible.forEach((entry) => {
      const row = document.createElement("tr");
      const nameCell = element("td", "evmsail-perf-function", entry.name);
      nameCell.title = entry.name;
      row.append(
        nameCell,
        comparisonCell(entry.exclusive_steps, maximum, active.index),
        element(
          "td",
          "evmsail-perf-number",
          `${entry.share_percent.toFixed(2)}%`,
        ),
      );
      body.append(row);
    });
    view.functions.replaceChildren(body);
    view.functionsHeader.replaceChildren(
      element("th", null, "Exact ELF symbol"),
      element("th", null, "Exclusive executed steps"),
      element("th", null, "Share of guest steps"),
    );
    view.functionTable.hidden = matches.length === 0;
    updateLimitNote(view.functionNote, matches.length, visible.length);
    view.functionsEmpty.hidden = matches.length !== 0;
    view.functionsEmpty.textContent = allFunctions.length
      ? "No executed function symbols match this filter."
      : `No executed function inventory was reported for ${active.name}.`;
  }

  function createView(root) {
    const measureBar = element("div", "evmsail-perf-measure");
    measureBar.setAttribute("role", "group");
    measureBar.setAttribute("aria-label", "Measure");
    measureBar.append(element("span", "evmsail-perf-measure__label", "Measure"));
    const measureOptions = element("div", "evmsail-perf-measure__options");
    const measureButtons = MEASURE_ORDER.map((id) => {
      const button = element(
        "button",
        "evmsail-perf-measure__option",
        MEASURES[id].label,
      );
      button.type = "button";
      button.dataset.measure = id;
      measureOptions.append(button);
      return button;
    });
    const measureNote = element("p", "evmsail-perf-measure__note");
    measureBar.append(measureOptions, measureNote);

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
    const measureRankingPanel = element("div", "evmsail-perf-ranking");
    const measureRankingTitle = element("h3", null, "By instruction steps");
    const measureRankingNote = element("p", "evmsail-perf-help");
    const measureRanking = element("div", "evmsail-perf-ranking__rows");
    measureRankingPanel.append(
      measureRankingTitle,
      measureRankingNote,
      measureRanking,
    );
    rankingGrid.append(measureRankingPanel);
    // The ranking is a browsing aid rather than the point of the page, so it
    // starts collapsed and the reader opts in.
    const rankingToggle = element(
      "button",
      "evmsail-perf-disclosure",
      "Show all fixture rankings",
    );
    rankingToggle.type = "button";
    rankingToggle.setAttribute("aria-expanded", "false");
    const rankingBody = element("div", "evmsail-perf-disclosure__body");
    rankingBody.hidden = true;
    rankingBody.append(
      element(
        "p",
        "evmsail-perf-help",
        "Sorted from high to low in the selected measure, annotated with each " +
          "fixture's block gas used. Select a row to load that comparison.",
      ),
      rankingGrid,
    );
    rankingToggle.addEventListener("click", () => {
      const open = rankingToggle.getAttribute("aria-expanded") === "true";
      rankingToggle.setAttribute("aria-expanded", open ? "false" : "true");
      rankingBody.hidden = open;
      rankingToggle.textContent = open
        ? "Show all fixture rankings"
        : "Hide fixture rankings";
    });
    rankingSection.append(rankingToggle, rankingBody);

    const summarySection = element("section", "evmsail-perf-section");
    const summaryHelp = element("p", "evmsail-perf-help");
    const summaryWrap = element("div", "evmsail-perf-table-wrap");
    const summaryTable = document.createElement("table");
    const summaryHead = document.createElement("thead");
    const summaryHeader = document.createElement("tr");
    const summaryPrimaryHeader = element("th", null, "Instruction steps");
    const summarySecondaryHeader = element("th", null, "Proving cost");
    summaryHeader.append(
      element("th", null, "Guest"),
      summaryPrimaryHeader,
      summarySecondaryHeader,
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
    const phasesTitle = element("h2", null, "Per-phase breakdown");
    const phasesHelp = element("p", "evmsail-perf-help");
    const phasesLegend = element("div", "evmsail-perf-legend");
    const phases = element("div", "evmsail-perf-phases");
    phasesSection.append(phasesTitle, phasesHelp, phasesLegend, phases);

    const detailSection = element("section", "evmsail-perf-section");
    const detailsHelp = element("p", "evmsail-perf-help");
    const detailWrap = element(
      "div",
      "evmsail-perf-table-wrap evmsail-perf-table-wrap--grid " +
        "evmsail-perf-table-wrap--scopes",
    );
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
      detailsHelp,
      detailWrap,
      detailsEmpty,
    );

    const operationSection = element("section", "evmsail-perf-section");
    const operationWrap = element(
      "div",
      "evmsail-perf-table-wrap evmsail-perf-table-wrap--grid " +
        "evmsail-perf-table-wrap--operations",
    );
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
        "Every costed operation used by at least one guest is shown. This " +
          "table is always proving cost: ZisK does not report instruction " +
          "steps per operation. Cost combines ordinary OP and FROP " +
          "execution. “Not used” means zero occurrences for this input, not " +
          "a truncated report. These are ZisK operations, not EVM opcodes.",
      ),
      operationWrap,
      operationsEmpty,
    );

    const functionSection = element("section", "evmsail-perf-section");
    const functionTabs = element("div", "evmsail-perf-tabs");
    functionTabs.setAttribute("role", "tablist");
    functionTabs.setAttribute("aria-label", "Guest");
    const functionPanel = element("div", "evmsail-perf-tabpanel");
    functionPanel.id = `evmsail-perf-functions-${Math.random()
      .toString(36)
      .slice(2, 10)}`;
    functionPanel.setAttribute("role", "tabpanel");
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
    const functionNote = element("small", "evmsail-perf-limit-note");
    functionControls.append(functionSearchField, functionNote);
    const functionMeasureNote = element("p", "evmsail-perf-help");
    functionMeasureNote.hidden = true;
    const functionsStatus = element(
      "p",
      "evmsail-perf-status evmsail-perf-status--warning",
    );
    functionsStatus.hidden = true;
    const functionWrap = element("div", "evmsail-perf-table-wrap");
    const functionTable = document.createElement("table");
    const functionHead = document.createElement("thead");
    const functionsHeader = document.createElement("tr");
    functionHead.append(functionsHeader);
    const functions = document.createElement("tbody");
    functionTable.append(functionHead, functions);
    functionWrap.append(functionTable);
    const functionsEmpty = element(
      "p",
      "evmsail-perf-empty",
      "No executed function symbols were reported for this guest.",
    );
    functionPanel.append(
      functionControls,
      functionMeasureNote,
      functionsStatus,
      functionWrap,
      functionsEmpty,
    );
    functionSection.append(
      element("h2", null, "Executed functions"),
      element(
        "p",
        "evmsail-perf-help",
        "Choose one guest tab to inspect every ELF symbol that executed at " +
          "least one ZisK instruction. Steps are exclusive to each symbol's " +
          "address range, so the rows partition that guest's total " +
          "instruction steps.",
      ),
      functionTabs,
      functionPanel,
    );

    root.replaceChildren(
      measureBar,
      controls,
      status,
      provenance,
      legend,
      rankingSection,
      summarySection,
      phasesSection,
      operationSection,
      detailSection,
      functionSection,
    );
    return {
      measureButtons,
      measureNote,
      fixtureSearch,
      fixtureSelect,
      fixtureNote,
      status,
      provenance,
      legend,
      inputBytes,
      measureRanking,
      measureRankingTitle,
      measureRankingNote,
      summary,
      summaryHelp,
      summaryPrimaryHeader,
      summarySecondaryHeader,
      phasesSection,
      phasesTitle,
      phasesHelp,
      phasesLegend,
      phases,
      details,
      detailsEmpty,
      detailsHelp,
      detailsHeader,
      operations,
      operationsEmpty,
      operationsHeader,
      functionTabs,
      functionPanel,
      functionSearch,
      functionNote,
      functionMeasureNote,
      functionsStatus,
      functionTable,
      functionsHeader,
      functions,
      functionsEmpty,
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
        "Generate it with python3 -m devtools.benchmarks.zisk --profile sdk --dashboard-dir … " +
        `(${error.message}).`;
      return;
    }

    const guests = catalog.guests || (catalog.guest ? [catalog.guest] : []);
    const fixtures = catalog.fixtures;
    let fixtureMatches = [];
    let shard = null;
    let aggregate = null;
    let pendingFixtureId = null;
    // Proving cost is the default: it is the resource a zkEVM guest is
  // actually charged for. Instruction steps remain one toggle away.
  let measure = MEASURES.cost;
    let functionGuest = guests[0]?.name;
    // The shared operation/scope grid reserves one column per guest, so its
    // minimum table width has to know how many guests the catalog carries.
    root.style.setProperty(
      "--evmsail-perf-guest-count",
      String(Math.max(1, guests.length)),
    );
    renderLegend(view, guests);

    function applyMeasureButtons() {
      view.measureButtons.forEach((button) => {
        const active = button.dataset.measure === measure.id;
        button.setAttribute("aria-pressed", active ? "true" : "false");
        button.classList.toggle(
          "evmsail-perf-measure__option--active",
          active,
        );
      });
      view.measureNote.textContent = measure.note;
    }

    function fixtureLabel(fixture) {
      const category = fixture.category ? `${fixture.category}/` : "";
      const gas = fixtureGas(fixture);
      return (
        `${category}${fixture.fixture} (${number.format(fixture.case_count)})` +
        (gas ? ` · ${gasLabel(gas, number)}` : "")
      );
    }

    function orderedByGas(items) {
      return [...items].sort((left, right) => {
        const leftGas = fixtureGas(left);
        const rightGas = fixtureGas(right);
        if (leftGas === null && rightGas === null) return 0;
        if (leftGas === null) return 1;
        if (rightGas === null) return -1;
        return (
          rightGas.value - leftGas.value ||
          fixtureLabel(left).localeCompare(fixtureLabel(right))
        );
      });
    }

    function currentFixture() {
      return fixtureMatches.find(
        (candidate) => candidate.id === view.fixtureSelect.value,
      );
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
      const fixture = currentFixture();
      renderRankings(
        view,
        catalog,
        guests,
        measure,
        fixture?.id,
        selectFixture,
      );
      if (!fixture) {
        shard = null;
        aggregate = null;
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
      aggregate = aggregateShard(shard, guests);
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
      renderSummary(view, aggregate, measure);
      renderPhases(view, aggregate, measure);
      renderDetails(view, aggregate, measure);
      renderOperations(view, aggregate);
      renderFunctions(
        view,
        aggregate,
        measure,
        functionGuest,
        selectFunctionGuest,
      );
    }

    function selectFunctionGuest(name) {
      functionGuest = name;
      if (!aggregate) return;
      const restoreFocus = view.functionTabs.contains(document.activeElement);
      renderFunctions(
        view,
        aggregate,
        measure,
        functionGuest,
        selectFunctionGuest,
      );
      if (restoreFocus) {
        view.functionTabs
          .querySelector('[aria-selected="true"]')
          ?.focus();
      }
    }

    function selectMeasure(id) {
      if (!MEASURES[id] || measure.id === id) return;
      measure = MEASURES[id];
      applyMeasureButtons();
      const fixture = currentFixture();
      renderRankings(
        view,
        catalog,
        guests,
        measure,
        fixture?.id,
        selectFixture,
      );
      if (fixture && shard) renderSelectedFixture(fixture);
    }

    view.measureButtons.forEach((button) => {
      button.addEventListener("click", () =>
        selectMeasure(button.dataset.measure),
      );
    });
    view.fixtureSearch.addEventListener("input", rebuildFixtures);
    view.fixtureSelect.addEventListener("change", loadFixture);
    view.functionSearch.addEventListener("input", () => {
      if (!aggregate) return;
      renderFunctions(
        view,
        aggregate,
        measure,
        functionGuest,
        selectFunctionGuest,
      );
    });

    applyMeasureButtons();
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
