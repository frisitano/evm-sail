(() => {
  "use strict";

  const OPTION_LIMIT = 200;
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
  const number = new Intl.NumberFormat("en-US");
  const METRICS = {
    cost: {
      label: "Proving cost",
      total: "total_cost",
      scopes: "scope_costs",
      inclusive: "Inclusive cost",
    },
    steps: {
      label: "Instruction steps",
      total: "total_steps",
      scopes: "scope_steps",
      inclusive: "Inclusive steps",
    },
  };

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

  function scopeFamily(name) {
    const match = SCOPE_FAMILIES.find(([pattern]) => pattern.test(name));
    return match ? match[1] : "Other";
  }

  function uniqueSorted(items, key) {
    return [...new Set(items.map(key))].sort((left, right) =>
      left.localeCompare(right, undefined, { numeric: true }),
    );
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

  function normalizeCase(profile, guests) {
    if (profile.guests) return profile;
    const guestName = guests[0]?.name;
    if (!guestName) return profile;
    return {
      id: profile.id,
      name: profile.name,
      block_index: profile.block_index,
      input_bytes: profile.input_bytes,
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

  function profileMetric(profile, guests, metricName) {
    const metric = METRICS[metricName];
    const guestValues = guests.map((guest, index) => {
      const guestProfile = profile.guests?.[guest.name] || {};
      return {
        ...guest,
        index,
        profile: guestProfile,
        totalValue: guestProfile[metric.total] || 0,
        scopeValues: guestProfile[metric.scopes] || {},
      };
    });
    return {
      ...metric,
      name: metricName,
      guestValues,
      maxTotal: Math.max(1, ...guestValues.map((guest) => guest.totalValue)),
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
        item.append(
          element(
            "span",
            `evmsail-perf-legend__swatch evmsail-perf-guest-${index}`,
          ),
          element("span", null, guest.name),
        );
        return item;
      }),
    );
  }

  function renderSummary(view, profile, metric) {
    const totals = metric.guestValues.map((guest) => guest.totalValue);
    const best = Math.min(...totals.filter((value) => value > 0));
    const body = document.createDocumentFragment();
    metric.guestValues.forEach((guest) => {
      const row = document.createElement("tr");
      const relative = best ? guest.totalValue / best : 0;
      row.append(
        element("td", null, guest.name),
        comparisonCell(
          guest.totalValue,
          metric.maxTotal,
          guest.index,
        ),
        element(
          "td",
          "evmsail-perf-number",
          relative ? `${relative.toFixed(2)}×` : "—",
        ),
        element(
          "td",
          "evmsail-perf-number",
          best ? `+${number.format(guest.totalValue - best)}` : "—",
        ),
      );
      body.append(row);
    });
    view.summary.replaceChildren(body);
    view.summaryMetricHeader.textContent = metric.label;
    view.summaryHelp.textContent =
      `All bars use the largest guest ${metric.label.toLocaleLowerCase()} ` +
      "as a shared scale. Relative and delta columns compare against the " +
      "least expensive guest for this exact input.";
    view.inputBytes.textContent = `${number.format(profile.input_bytes)} input bytes`;
  }

  function renderProvenance(view, catalog, fixture, profile, guests) {
    const generated = new Date(
      Number(catalog.generated_at_unix_ns) / 1_000_000,
    );
    view.provenance.textContent =
      `${guests.map((guest) => guest.name).join(" · ")} · ${fixture.path} · ` +
      `${profile.name} / block ${profile.block_index} · generated ` +
      generated.toISOString().slice(0, 10);
  }

  function renderPipeline(view, metric) {
    const fragment = document.createDocumentFragment();
    PIPELINE.forEach(([label, scope, depth]) => {
      const values = metric.guestValues.map((guest) =>
        scope
          ? Object.hasOwn(guest.scopeValues, scope)
            ? guest.scopeValues[scope]
            : null
          : guest.totalValue,
      );
      if (scope && values.every((value) => value === null)) return;
      const row = element("div", "evmsail-perf-scope");
      row.style.setProperty("--scope-depth", depth);
      row.append(element("div", "evmsail-perf-scope__name", label));
      const comparisons = element("div", "evmsail-perf-scope__comparisons");
      values.forEach((value, index) => {
        const guest = metric.guestValues[index];
        const item = element("div", "evmsail-perf-scope__guest");
        item.append(
          element("span", "evmsail-perf-scope__guest-name", guest.name),
          element(
            "span",
            value === null
              ? "evmsail-perf-unavailable"
              : "evmsail-perf-number",
            value === null ? "Not instrumented" : number.format(value),
          ),
        );
        if (value !== null) {
          item.append(bar((value / metric.maxTotal) * 100, guest.index));
        }
        comparisons.append(item);
      });
      row.append(comparisons);
      fragment.append(row);
    });
    view.pipeline.replaceChildren(fragment);
    view.pipelineHelp.textContent =
      "Indentation shows logical containment. Every guest bar uses the same " +
      `absolute ${metric.label.toLocaleLowerCase()} scale. Scope totals are ` +
      "inclusive; missing tags are shown as not instrumented, never as zero.";
  }

  function renderDetails(view, metric) {
    const scopeNames = new Set();
    metric.guestValues.forEach((guest) => {
      Object.keys(guest.scopeValues).forEach((name) => scopeNames.add(name));
    });
    const rows = [...scopeNames]
      .map((name) => ({
        name,
        values: metric.guestValues.map((guest) =>
          Object.hasOwn(guest.scopeValues, name)
            ? guest.scopeValues[name]
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
        row.append(comparisonCell(value, maximum, index));
      });
      body.append(row);
    });
    view.details.replaceChildren(body);
    view.detailsEmpty.hidden = rows.length !== 0;
    view.detailsHeader.replaceChildren(
      element("th", null, "Scope"),
      element("th", null, "Family"),
      ...metric.guestValues.map((guest) =>
        element("th", null, guest.name),
      ),
    );
  }

  function renderFunctions(view, profile, guests) {
    const guestIndex = Math.max(
      0,
      guests.findIndex((guest) => guest.name === view.functionGuestSelect.value),
    );
    const guest = guests[guestIndex];
    const guestProfile = profile.guests?.[guest?.name] || {};
    const allFunctions = guestProfile.executed_functions || [];
    const needle = view.functionSearch.value.trim().toLocaleLowerCase();
    const rows = needle
      ? allFunctions.filter((entry) =>
          entry.name.toLocaleLowerCase().includes(needle),
        )
      : allFunctions;
    const maximum = Math.max(
      1,
      ...allFunctions.map((entry) => entry.exclusive_steps || 0),
    );
    const body = document.createDocumentFragment();
    rows.forEach((entry) => {
      const row = document.createElement("tr");
      const nameCell = element("td", "evmsail-perf-function", entry.name);
      nameCell.title = entry.name;
      row.append(
        nameCell,
        comparisonCell(entry.exclusive_steps, maximum, guestIndex),
        element(
          "td",
          "evmsail-perf-number",
          `${entry.share_percent.toFixed(2)}%`,
        ),
      );
      body.append(row);
    });
    view.functions.replaceChildren(body);
    view.functionsEmpty.hidden = rows.length !== 0;
    view.functionsEmpty.textContent = allFunctions.length
      ? "No executed function symbols match this filter."
      : `No executed function inventory was reported for ${guest?.name || "this guest"}.`;
    updateLimitNote(view.functionNote, rows.length, rows.length);
    const stackMismatch =
      guestProfile.function_profile_status === "stack_mismatch";
    view.functionsStatus.hidden = !stackMismatch;
    view.functionsStatus.textContent = stackMismatch
      ? "ZisK could not maintain an inclusive call stack for this guest. " +
        "The table remains complete because it assigns each executed " +
        "instruction to its containing ELF symbol instead."
      : "";
    view.functionsHeader.replaceChildren(
      element("th", null, "Exact ELF symbol"),
      element("th", null, "Exclusive executed steps"),
      element("th", null, "Share of guest steps"),
    );
  }

  function renderOperations(view, profile, guests) {
    const byName = new Map();
    guests.forEach((guest, guestIndex) => {
      const operations = profile.guests?.[guest.name]?.opcode_costs || [];
      operations.forEach((entry) => {
        if (!byName.has(entry.name)) {
          byName.set(entry.name, Array(guests.length).fill(null));
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
          row.append(comparisonCell(null, maximum, index, "", "Not used"));
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
      ...guests.map((guest) => element("th", null, guest.name)),
    );
  }

  function createView(root) {
    const controls = element("div", "evmsail-perf-controls");
    const selectors = {};
    [
      ["suite", "Corpus type"],
      ["target", "Fork target"],
      ["fork", "Source fork"],
      ["feature", "Feature / EIP"],
    ].forEach(([key, label]) => {
      const field = element("label", "evmsail-perf-field");
      field.append(element("span", null, label));
      selectors[key] = document.createElement("select");
      field.append(selectors[key]);
      controls.append(field);
    });

    const metricField = element("label", "evmsail-perf-field");
    metricField.append(element("span", null, "Measure"));
    const metricSelect = document.createElement("select");
    Object.entries(METRICS).forEach(([value, metric]) => {
      const option = element("option", null, metric.label);
      option.value = value;
      metricSelect.append(option);
    });
    metricSelect.value = "steps";
    metricField.append(metricSelect);
    controls.append(metricField);

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

    const caseSearch = document.createElement("input");
    caseSearch.type = "search";
    caseSearch.placeholder = "Filter test name or block";
    const caseSelect = document.createElement("select");
    const caseNote = element("small", "evmsail-perf-limit-note");
    const caseField = element(
      "label",
      "evmsail-perf-field evmsail-perf-field--wide",
    );
    caseField.append(
      element("span", null, "Embedded case / block"),
      caseSearch,
      caseSelect,
      caseNote,
    );
    controls.append(caseField);

    const status = element(
      "div",
      "evmsail-perf-status",
      "Loading ZisK profile catalog…",
    );
    status.setAttribute("role", "status");
    const provenance = element("p", "evmsail-perf-provenance");
    const legend = element("div", "evmsail-perf-legend");
    const inputBytes = element("span", "evmsail-perf-input");

    const summarySection = element("section", "evmsail-perf-section");
    const summaryHelp = element("p", "evmsail-perf-help");
    const summaryWrap = element("div", "evmsail-perf-table-wrap");
    const summaryTable = document.createElement("table");
    const summaryHead = document.createElement("thead");
    const summaryHeader = document.createElement("tr");
    const summaryMetricHeader = element("th", null, "Instruction steps");
    summaryHeader.append(
      element("th", null, "Guest"),
      summaryMetricHeader,
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
    const functionGuestField = element("label", "evmsail-perf-field");
    functionGuestField.append(element("span", null, "Guest"));
    const functionGuestSelect = document.createElement("select");
    functionGuestField.append(functionGuestSelect);
    const functionSearchField = element("label", "evmsail-perf-field");
    functionSearchField.append(element("span", null, "Filter symbols"));
    const functionSearch = document.createElement("input");
    functionSearch.type = "search";
    functionSearch.placeholder = "Function or symbol name";
    functionSearchField.append(functionSearch);
    const functionNote = element("small", "evmsail-perf-limit-note");
    functionControls.append(
      functionGuestField,
      functionSearchField,
      functionNote,
    );
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
    const functionsStatus = element(
      "p",
      "evmsail-perf-status evmsail-perf-status--warning",
    );
    functionsStatus.hidden = true;
    functionSection.append(
      element("h2", null, "Executed functions"),
      element(
        "p",
        "evmsail-perf-help",
        "Choose one guest to inspect every ELF symbol that executed at least one ZisK instruction. Steps are exclusive to each symbol's address range, so the rows partition the guest's total instruction steps.",
      ),
      functionControls,
      functionsStatus,
      functionWrap,
      functionsEmpty,
    );

    root.replaceChildren(
      controls,
      status,
      provenance,
      legend,
      summarySection,
      operationSection,
      pipelineSection,
      detailSection,
      functionSection,
    );
    return {
      selectors,
      metricSelect,
      fixtureSearch,
      fixtureSelect,
      fixtureNote,
      caseSearch,
      caseSelect,
      caseNote,
      status,
      provenance,
      legend,
      inputBytes,
      summary,
      summaryHelp,
      summaryMetricHeader,
      pipeline,
      pipelineHelp,
      details,
      detailsEmpty,
      detailsHeader,
      operations,
      operationsEmpty,
      operationsHeader,
      functionGuestSelect,
      functionSearch,
      functionNote,
      functions,
      functionsEmpty,
      functionsStatus,
      functionsHeader,
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
    const selected = {};
    let fixtureMatches = [];
    let shard = null;
    let caseMatches = [];
    renderLegend(view, guests);
    replaceOptions(
      view.functionGuestSelect,
      guests.map((guest) => guest.name),
      guests[0]?.name,
      "Function guest",
      (name) => name,
    );

    function rebuildGroups(changed) {
      const keys = ["suite", "target", "fork", "feature"];
      const changedIndex = Math.max(0, keys.indexOf(changed));
      for (let index = changedIndex; index < keys.length; index += 1) {
        const key = keys[index];
        const eligible = fixtures.filter((fixture) =>
          keys
            .slice(0, index)
            .every((parent) => fixture[parent] === selected[parent]),
        );
        const values = uniqueSorted(eligible, (fixture) => fixture[key]);
        const current = view.selectors[key].value || selected[key];
        selected[key] = values.includes(current) ? current : values[0];
        replaceOptions(view.selectors[key], values, selected[key], key);
      }
      rebuildFixtures();
    }

    function fixtureLabel(fixture) {
      const category = fixture.category ? `${fixture.category}/` : "";
      return `${category}${fixture.fixture} (${number.format(fixture.case_count)})`;
    }

    function rebuildFixtures() {
      const grouped = fixtures.filter(
        (fixture) =>
          fixture.suite === selected.suite &&
          fixture.target === selected.target &&
          fixture.fork === selected.fork &&
          fixture.feature === selected.feature,
      );
      const result = filteredOptions(
        grouped,
        view.fixtureSearch.value,
        fixtureLabel,
      );
      fixtureMatches = result.matches;
      const previous = view.fixtureSelect.value;
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

    async function loadFixture() {
      const fixture = fixtureMatches.find(
        (candidate) => candidate.id === view.fixtureSelect.value,
      );
      if (!fixture) {
        view.caseSelect.replaceChildren();
        view.status.textContent = "No fixture matches the current filters.";
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
      rebuildCases();
    }

    function caseLabel(profile) {
      return (
        `${profile.name} · block ${profile.block_index} · ` +
        `${number.format(profile.input_bytes)} input bytes`
      );
    }

    function rebuildCases() {
      const result = filteredOptions(
        shard.cases,
        view.caseSearch.value,
        caseLabel,
      );
      caseMatches = result.matches;
      const previous = view.caseSelect.value;
      replaceOptions(
        view.caseSelect,
        result.visible.map((profile) => profile.id),
        result.visible.some((profile) => profile.id === previous)
          ? previous
          : result.visible[0]?.id,
        "Embedded case and block",
        (id) => caseLabel(shard.cases.find((profile) => profile.id === id)),
      );
      updateLimitNote(view.caseNote, result.matches.length, result.visible.length);
      renderSelectedCase();
    }

    function renderSelectedCase() {
      const profile = caseMatches.find(
        (candidate) => candidate.id === view.caseSelect.value,
      );
      const fixture = fixtures.find(
        (candidate) => candidate.id === view.fixtureSelect.value,
      );
      if (!profile || !fixture) {
        view.status.textContent = "No embedded case matches the current filter.";
        return;
      }
      view.status.textContent =
        `${number.format(catalog.fixture_count)} fixture files · ` +
        `${number.format(catalog.case_count)} profiled cases · ` +
        `${guests.length} guests`;
      const metric = profileMetric(profile, guests, view.metricSelect.value);
      renderProvenance(view, catalog, fixture, profile, guests);
      renderSummary(view, profile, metric);
      renderPipeline(view, metric);
      renderDetails(view, metric);
      renderOperations(view, profile, guests);
      renderFunctions(view, profile, guests);
    }

    ["suite", "target", "fork", "feature"].forEach((key) => {
      view.selectors[key].addEventListener("change", () => {
        selected[key] = view.selectors[key].value;
        rebuildGroups(key);
      });
    });
    view.fixtureSearch.addEventListener("input", rebuildFixtures);
    view.fixtureSelect.addEventListener("change", loadFixture);
    view.caseSearch.addEventListener("input", rebuildCases);
    view.caseSelect.addEventListener("change", renderSelectedCase);
    view.metricSelect.addEventListener("change", renderSelectedCase);
    view.functionGuestSelect.addEventListener("change", renderSelectedCase);
    view.functionSearch.addEventListener("input", renderSelectedCase);

    rebuildGroups("suite");
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
