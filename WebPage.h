#ifndef WEBRADIO_WEBPAGE_H
#define WEBRADIO_WEBPAGE_H
#include <Arduino.h>
static const char WEB_PAGE[] PROGMEM = R"WEBRADIO(<!doctype html>
<html lang="de">
  <head>
    <meta charset="utf-8" />
    <meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover" />
    <meta name="theme-color" content="#163e39" />
    <title>Webradio · Deine Sender</title>
    <style>
      :root {
        color-scheme: light;
        --ink: #173c37;
        --muted: #60736d;
        --paper: #f5f4ee;
        --line: #dce3db;
        --accent: #d9f086;
        --red: #a6382b;
      }
      * {
        box-sizing: border-box;
      }
      body {
        margin: 0;
        background: var(--paper);
        color: var(--ink);
        font:
          16px/1.5 system-ui,
          -apple-system,
          BlinkMacSystemFont,
          "Segoe UI",
          sans-serif;
      }
      button,
      input,
      select {
        font: inherit;
      }
      button {
        cursor: pointer;
        min-height: 46px;
        padding: 10px 16px;
        border: 1px solid var(--line);
        border-radius: 12px;
        background: #fff;
        color: var(--ink);
        font-weight: 650;
      }
      button:hover {
        background: #edf2e9;
      }
      button:disabled {
        opacity: 0.4;
        cursor: default;
      }
      button:focus-visible,
      input:focus-visible,
      select:focus-visible,
      summary:focus-visible {
        outline: 3px solid #648d20;
        outline-offset: 3px;
      }
      .primary {
        background: var(--ink);
        color: white;
        border-color: var(--ink);
      }
      .primary:hover {
        background: #28574d;
      }
      .quiet {
        background: transparent;
      }
      .danger {
        color: var(--red);
      }
      .wrap {
        max-width: 1040px;
        margin: auto;
        padding: 0 24px;
      }
      main.wrap {
        padding-bottom: 170px;
      }
      header {
        border-bottom: 1px solid var(--line);
      }
      .nav {
        display: flex;
        align-items: center;
        justify-content: space-between;
        min-height: 86px;
        gap: 12px;
      }
      .brand {
        display: flex;
        align-items: center;
        gap: 12px;
        font-size: 18px;
        font-weight: 800;
        letter-spacing: 0.04em;
      }
      .logo {
        display: flex;
        align-items: center;
        justify-content: center;
        gap: 3px;
        width: 40px;
        height: 40px;
        border-radius: 12px;
        background: var(--ink);
      }
      .logo i {
        display: block;
        width: 3px;
        background: var(--accent);
        border-radius: 3px;
        height: 12px;
      }
      .logo i:nth-child(2),
      .logo i:nth-child(4) {
        height: 20px;
      }
      .logo i:nth-child(3) {
        height: 28px;
      }
      .connection {
        font-size: 13px;
        color: var(--muted);
        display: flex;
        align-items: center;
        gap: 7px;
      }
      .dot {
        width: 7px;
        height: 7px;
        border-radius: 50%;
        background: #8b958c;
      }
      .connection.ready .dot {
        background: #57832a;
        box-shadow: 0 0 0 4px #e4edda;
      }
      .hero {
        padding: 44px 0 28px;
        display: flex;
        justify-content: space-between;
        gap: 24px;
        align-items: flex-end;
      }
      .eyebrow {
        text-transform: uppercase;
        letter-spacing: 0.15em;
        font-size: 11px;
        font-weight: 800;
        color: var(--muted);
        margin: 0 0 12px;
      }
      h1 {
        font-size: clamp(34px, 6vw, 52px);
        line-height: 1.1;
        letter-spacing: -0.05em;
        margin: 0 0 16px;
        font-weight: 750;
      }
      .intro {
        margin: 0;
        color: var(--muted);
        max-width: 540px;
      }
      .count {
        white-space: nowrap;
        align-self: flex-start;
        padding: 12px 16px;
        border: 1px solid var(--line);
        border-radius: 16px;
        background: #eeeee4;
        color: var(--muted);
        font-size: 13px;
      }
      .count strong {
        font-size: 24px;
        color: var(--ink);
        margin-right: 6px;
      }
      .settings-bar {
        display: grid;
        grid-template-columns: repeat(2, minmax(0, 1fr));
        gap: 10px;
        margin-bottom: 20px;
      }
      .settings-card {
        min-width: 0;
        text-align: left;
        background: #eef2e8;
        padding: 14px 16px;
      }
      .settings-card strong,
      .settings-card small {
        display: block;
        overflow-wrap: anywhere;
      }
      .settings-card strong {
        font-size: 14px;
      }
      .settings-card small {
        font-size: 12px;
        color: var(--muted);
        font-weight: 400;
        margin-top: 4px;
      }
      .group-row {
        display: flex;
        align-items: center;
        gap: 12px;
        min-width: 0;
        padding: 12px;
        background: #fff;
        border: 1px solid var(--line);
        border-radius: 16px;
      }
      .group-arrow {
        padding: 8px;
        width: 44px;
        font-size: 19px;
      }
      .group-edit {
        font-size: 13px;
      }
      #group-manager {
        max-width: 700px;
      }
      .group-toolbar {
        display: flex;
        align-items: center;
        gap: 12px;
        margin: 18px 0;
      }
      .group-toolbar span {
        flex: 1;
        font-size: 12px;
        color: var(--muted);
      }
      .dialog-intro {
        color: var(--muted);
        font-size: 13px;
      }
      #group-error,
      #hostname-error {
        color: var(--red);
        font-size: 13px;
      }
      .save-inner > div:first-child {
        min-width: 0;
        flex: 1;
      }
      .save-hint {
        overflow-wrap: anywhere;
      }
      #hostname-preview {
        overflow-wrap: anywhere;
      }
      #message a {
        color: inherit;
        font-weight: 700;
        overflow-wrap: anywhere;
      }
      .tools {
        display: flex;
        gap: 12px;
        flex-wrap: wrap;
        margin-bottom: 20px;
      }
      .tools input {
        flex: 1;
        min-width: 170px;
      }
      .tools select {
        max-width: 260px;
      }
      .tools button {
        background: var(--accent);
        border-color: #c8df75;
      }
      input,
      select {
        width: 100%;
        min-height: 48px;
        padding: 11px 13px;
        border: 1px solid #cbd6cd;
        border-radius: 12px;
        background: white;
        color: var(--ink);
      }
      fieldset {
        border: 0;
        padding: 0;
        margin: 0;
        min-width: 0;
      }
      .list-caption {
        display: flex;
        justify-content: space-between;
        font-size: 12px;
        color: var(--muted);
        padding: 0 4px 12px;
        gap: 12px;
      }
      .stations {
        grid-template-columns: minmax(0, 1fr);
        list-style: none;
        margin: 0;
        padding: 0;
        display: grid;
        gap: 10px;
      }
      .station {
        min-width: 0;
        display: flex;
        align-items: center;
        gap: 16px;
        padding: 18px;
        background: #fff;
        border: 1px solid var(--line);
        border-radius: 18px;
        transition: border-color 0.15s;
      }
      .station:hover {
        border-color: #93b0a1;
      }
      .number {
        font-variant-numeric: tabular-nums;
        color: #6d8277;
        font-size: 13px;
        width: 30px;
        flex-shrink: 0;
      }
      .station-info {
        flex: 1;
        min-width: 0;
      }
      .station-name {
        font-size: 17px;
        font-weight: 750;
        overflow-wrap: anywhere;
      }
      .badge {
        display: inline-block;
        font-size: 11px;
        border-radius: 6px;
        padding: 3px 7px;
        background: #eef2e8;
        color: #4b6357;
        margin-top: 5px;
        max-width: 100%;
        overflow-wrap: anywhere;
      }
      .url {
        font-size: 12px;
        color: var(--muted);
        white-space: nowrap;
        overflow: hidden;
        text-overflow: ellipsis;
        margin-top: 6px;
      }
      .actions {
        display: flex;
        align-items: center;
        gap: 6px;
      }
      .arrow {
        padding: 8px;
        width: 44px;
        font-size: 19px;
      }
      .edit {
        margin-left: 6px;
        font-size: 13px;
      }
      .empty {
        border: 1px dashed #bac9ba;
        border-radius: 18px;
        padding: 36px 20px;
        text-align: center;
        color: var(--muted);
        background: #fafbf6;
      }
      #message {
        padding: 14px 17px;
        border: 1px solid #c7d9b0;
        background: #edf5df;
        border-radius: 12px;
        margin-bottom: 18px;
        overflow-wrap: anywhere;
      }
      #message.error {
        background: #fceee8;
        border-color: #e7b3a5;
        color: #872d23;
      }
      #message:empty {
        display: none;
      }
      .savebar {
        position: fixed;
        left: 0;
        right: 0;
        bottom: 0;
        padding: 16px 0 max(16px, env(safe-area-inset-bottom));
        background: #f5f4eef5;
        border-top: 1px solid var(--line);
        backdrop-filter: blur(12px);
        z-index: 2;
      }
      .save-inner {
        max-width: 1040px;
        margin: 0 auto;
        padding: 0 24px;
        display: flex;
        justify-content: space-between;
        align-items: center;
        gap: 12px;
      }
      .save-state {
        font-size: 14px;
        font-weight: 650;
      }
      .save-hint {
        font-size: 12px;
        color: var(--muted);
        margin-top: 3px;
      }
      .save-buttons {
        display: flex;
        gap: 8px;
      }
      .changed {
        display: inline-block;
        background: var(--accent);
        width: 8px;
        height: 8px;
        border-radius: 50%;
        margin-right: 7px;
      }
      .help {
        padding: 24px 0 35px;
        color: var(--muted);
        font-size: 13px;
        max-width: 760px;
      }
      summary {
        cursor: pointer;
        font-weight: 650;
        min-height: 36px;
      }
      dialog {
        border: 1px solid var(--line);
        border-radius: 24px;
        padding: 26px;
        max-width: 520px;
        width: calc(100% - 28px);
        max-height: calc(100dvh - 28px);
        overflow: auto;
        color: var(--ink);
        box-shadow: 0 24px 100px #123c3738;
      }
      dialog::backdrop {
        background: #142e286b;
        backdrop-filter: blur(4px);
      }
      .modal-head {
        display: flex;
        align-items: center;
        justify-content: space-between;
        gap: 10px;
        margin-bottom: 20px;
      }
      h2 {
        font-size: 24px;
        letter-spacing: -0.03em;
        margin: 0;
      }
      .close {
        border: 0;
        font-size: 24px;
        padding: 2px;
        width: 46px;
      }
      .field {
        display: block;
        margin: 16px 0;
      }
      .label {
        display: block;
        font-size: 13px;
        font-weight: 700;
        margin-bottom: 7px;
      }
      .field small {
        display: block;
        font-size: 12px;
        color: var(--muted);
        margin-top: 6px;
      }
      .dialog-actions {
        display: flex;
        justify-content: flex-end;
        gap: 8px;
        margin-top: 24px;
      }
      .dialog-actions .danger {
        margin-right: auto;
      }
      #form-error {
        font-size: 13px;
        color: var(--red);
        overflow-wrap: anywhere;
      }
      [hidden] {
        display: none !important;
      }
      @media (max-width: 640px) {
        .settings-bar {
          grid-template-columns: minmax(0, 1fr);
        }
        .group-row {
          flex-wrap: wrap;
        }
        .group-row .station-info {
          flex-basis: 100%;
        }
        .group-row .actions {
          margin: 0;
          width: 100%;
        }
        .group-edit {
          margin-left: auto;
        }
        .group-toolbar {
          flex-wrap: wrap;
        }
        .group-toolbar span {
          flex-basis: 100%;
        }

        .wrap {
          padding: 0 16px;
        }
        .nav {
          min-height: 72px;
        }
        .connection {
          font-size: 11px;
          max-width: 130px;
        }
        .brand {
          font-size: 15px;
        }
        .hero {
          padding: 30px 0 24px;
          display: block;
        }
        .count {
          display: inline-block;
          margin-top: 18px;
          padding: 7px 12px;
        }
        .count strong {
          font-size: 19px;
        }
        .tools {
          gap: 8px;
        }
        .tools input {
          flex-basis: 100%;
        }
        .tools select {
          flex: 1;
          max-width: none;
          min-width: 0;
          width: 0;
        }
        .tools button {
          padding: 10px 12px;
        }
        .station {
          gap: 10px;
          padding: 14px 12px;
          flex-wrap: wrap;
        }
        .number {
          width: 24px;
        }
        .station-info {
          flex-basis: calc(100% - 40px);
        }
        .actions {
          margin-left: 34px;
          width: calc(100% - 34px);
        }
        .edit {
          margin-left: auto;
        }
        .station-name {
          font-size: 16px;
        }
        .save-inner {
          padding: 0 16px;
          flex-wrap: wrap;
        }
        .save-buttons {
          width: 100%;
        }
        .save-buttons button {
          flex: 1;
        }
        .list-caption {
          font-size: 11px;
        }
        dialog {
          padding: 20px;
        }
        .dialog-actions {
          flex-wrap: wrap;
        }
        .dialog-actions button {
          flex: 1;
        }
        .dialog-actions .danger {
          flex-basis: 100%;
          margin: 0;
        }
        .help {
          padding-bottom: 22px;
        }
      }
      @media (prefers-reduced-motion: reduce) {
        * {
          transition: none !important;
        }
      }
    </style>
  </head>
  <body>
    <header>
      <div class="wrap nav">
        <div class="brand">
          <span class="logo" aria-hidden="true"><i></i><i></i><i></i><i></i><i></i></span>WEBRADIO
        </div>
        <div id="connection" class="connection">
          <span class="dot"></span><span id="connection-text">Verbindung wird hergestellt</span>
        </div>
      </div>
    </header>
    <main class="wrap">
      <section class="hero">
        <div>
          <p class="eyebrow">Deine Musik. Dein Radio.</p>
          <h1>Lieblingssender,<br />in deiner Reihenfolge.</h1>
          <p class="intro">
            Entdeckt, gesammelt, gut sortiert. Hier findet jeder Sender seinen Platz auf deinem
            Radio.
          </p>
        </div>
        <div class="count"><strong id="total">–</strong> Sender</div>
      </section>
      <div id="message" role="status" aria-live="polite"></div>
      <fieldset id="workspace" disabled>
        <legend hidden>Senderverwaltung</legend>
        <div class="settings-bar">
          <button id="hostname-open" type="button" class="settings-card">
            <strong id="hostname-display">webradio.local</strong><small>Hostname bearbeiten</small>
          </button>
          <button id="groups-open" type="button" class="settings-card">
            <strong>Gruppen verwalten</strong
            ><small id="group-total">Namen, Untergruppen und Reihenfolge</small>
          </button>
        </div>
        <div class="tools">
          <input
            id="search"
            type="search"
            placeholder="Sender suchen …"
            aria-label="Sender suchen"
            autocomplete="off"
          /><select id="filter" aria-label="Nach Gruppe filtern">
            <option value="">Alle Gruppen</option></select
          ><button id="add" type="button">+ Sender hinzufügen</button>
        </div>
        <div class="list-caption">
          <span id="visible-count">Sender werden geladen …</span><span>Mit ↑ ↓ sortieren</span>
        </div>
        <ol id="stations" class="stations"></ol>
        <div id="empty" class="empty" hidden>
          Keine passenden Sender. Versuche einen anderen Suchbegriff oder eine andere Gruppe.
        </div>
      </fieldset>
      <details class="help">
        <summary>So funktioniert deine Senderliste</summary>
        <p>
          Bearbeite Namen, Stream-URL und Gruppe eines Senders über „Bearbeiten“. Mit den Pfeilen
          verschiebst du ihn vor oder hinter den nächsten sichtbaren Sender. Im Bearbeitungsfenster
          kannst du auch direkt eine Position angeben.
        </p>
        <p>
          Erst „Änderungen speichern“ überträgt die gesamte Liste auf dein Radio. Sie bleibt nach
          dem Ausschalten erhalten. Die Reihenfolge gilt auch innerhalb der Gruppen im
          Drehencoder-Menü. Wird der laufende Sender gelöscht, startet der erste Sender der neuen
          Liste.
        </p>
        <p>
          Nutze direkte HTTP- oder HTTPS-Streams (MP3/AAC); auch M3U/PLS-Playlists sind möglich. Bis
          zu 48 Sender, 24 Gruppen einschließlich Startseite und insgesamt 8 KB Konfiguration.
          Gruppen und Untergruppen bearbeitest du über „Gruppen verwalten“. Ein neuer Hostname wird
          mit „Speichern &amp; neu starten“ aktiviert.
        </p>
      </details>
      <div class="savebar">
        <div class="save-inner">
          <div>
            <div id="save-state" class="save-state">Liste wird geladen</div>
            <div id="save-hint" class="save-hint">Bleibt auch nach dem Ausschalten erhalten</div>
          </div>
          <div class="save-buttons">
            <button id="reload" class="quiet" type="button">Neu laden</button
            ><button id="save" class="primary" type="button" disabled>Änderungen speichern</button>
          </div>
        </div>
      </div>
    </main>
    <dialog id="editor" aria-labelledby="editor-title">
      <form id="station-form">
        <div class="modal-head">
          <h2 id="editor-title">Sender bearbeiten</h2>
          <button class="close" id="close-editor" type="button" aria-label="Schließen">×</button>
        </div>
        <label class="field"
          ><span class="label">Sendername</span
          ><input
            id="name"
            required
            maxlength="63"
            placeholder="z. B. Mein Lieblingsradio"
            autocomplete="off"
          /><small>Bis zu 63 Zeichen. Umlaute zählen doppelt.</small></label
        ><label class="field"
          ><span class="label">Stream-URL</span
          ><input
            id="url"
            type="url"
            required
            maxlength="255"
            placeholder="https://…/stream.mp3"
            inputmode="url"
            autocomplete="off"
            spellcheck="false"
          /><small>Direkter HTTP(S)-Stream oder M3U/PLS-Playlist.</small></label
        ><label class="field"
          ><span class="label">Gruppe</span
          ><select id="group" required></select></label
        ><label class="field"
          ><span class="label">Position in der gesamten Liste</span
          ><input id="position" type="number" min="1" step="1" required inputmode="numeric"
        /></label>
        <p id="form-error" role="alert"></p>
        <div class="dialog-actions">
          <button id="delete" type="button" class="danger">Sender löschen</button
          ><button id="cancel-editor" type="button">Abbrechen</button
          ><button type="submit" class="primary">Übernehmen</button>
        </div>
      </form>
    </dialog>

    <dialog id="hostname-editor" aria-labelledby="hostname-title">
      <form id="hostname-form">
        <div class="modal-head">
          <h2 id="hostname-title">Dein Radio im Netzwerk</h2>
          <button id="hostname-close" type="button" class="close" aria-label="Schließen">×</button>
        </div>
        <label class="field"
          ><span class="label">Hostname</span
          ><input
            id="hostname-input"
            required
            maxlength="31"
            placeholder="radio-kueche"
            autocomplete="off"
            autocapitalize="none"
            spellcheck="false"
          /><small
            >1–31 Zeichen: a–z, Ziffern und Bindestriche innerhalb des Namens. Ohne .local
            eingeben.</small
          ></label
        >
        <p class="dialog-intro">Neue Adresse: <strong id="hostname-preview"></strong></p>
        <p class="dialog-intro">
          Der Name gilt für WLAN und die .local-Adresse. Beim Speichern einer Namensänderung startet
          das Radio neu; die Wiedergabe wird kurz unterbrochen.
        </p>
        <p id="hostname-error" role="alert"></p>
        <div class="dialog-actions">
          <button id="hostname-cancel" type="button">Abbrechen</button
          ><button class="primary" type="submit">Übernehmen</button>
        </div>
      </form>
    </dialog>
    <dialog id="group-manager" aria-labelledby="groups-title">
      <div class="modal-head">
        <h2 id="groups-title">Deine Gruppen</h2>
        <button id="groups-close" type="button" class="close" aria-label="Schließen">×</button>
      </div>
      <p class="dialog-intro">
        Ordne deine Musik in Gruppen und Untergruppen. Die Pfeile bestimmen die Reihenfolge im
        Radio-Menü. Die Startseite bleibt immer erhalten.
      </p>
      <div class="group-toolbar">
        <span id="groups-count"></span
        ><button id="group-add" type="button">+ Gruppe hinzufügen</button>
      </div>
      <ol id="group-list" class="stations"></ol>
      <p class="dialog-intro">
        Änderungen werden erst mit dem Speicherknopf auf das Radio übertragen.
      </p>
      <div class="dialog-actions">
        <button id="groups-done" class="primary" type="button">Fertig</button>
      </div>
    </dialog>
    <dialog id="group-editor" aria-labelledby="group-title">
      <form id="group-form">
        <div class="modal-head">
          <h2 id="group-title">Gruppe bearbeiten</h2>
          <button id="group-close" type="button" class="close" aria-label="Schließen">×</button>
        </div>
        <label class="field"
          ><span class="label">Gruppenname</span
          ><input id="group-name" required maxlength="63" autocomplete="off" /><small
            >Bis zu 63 Zeichen. Umlaute zählen doppelt.</small
          ></label
        >
        <label class="field"
          ><span class="label">Übergeordnete Gruppe</span
          ><select id="group-parent" required></select
          ><small>Eigene Untergruppen stehen nicht als Eltern zur Auswahl.</small></label
        >
        <label class="field"
          ><span class="label">Position in der Gruppenliste</span
          ><input id="group-position" required type="number" min="2" step="1" inputmode="numeric"
        /></label>
        <p id="group-error" role="alert"></p>
        <div class="dialog-actions">
          <button id="group-delete" class="danger" type="button">Gruppe löschen</button
          ><button id="group-cancel" type="button">Abbrechen</button
          ><button class="primary" type="submit">Übernehmen</button>
        </div>
      </form>
    </dialog>
    <dialog id="confirm" aria-labelledby="confirm-title">
      <h2 id="confirm-title">Änderung bestätigen</h2>
      <p id="confirm-text"></p>
      <div class="dialog-actions">
        <button id="confirm-no" type="button">Abbrechen</button
        ><button id="confirm-yes" class="primary" type="button">Bestätigen</button>
      </div>
    </dialog>
    <script>
      "use strict";
      const $ = (id) => document.getElementById(id);
      let rows = [],
        groups = [],
        hostname = "webradio",
        activeHostname = "webradio",
        original = "",
        token = "",
        revision = 0,
        loaded = false,
        busy = false,
        restarting = false,
        editIndex = -1,
        editGroupId = null,
        nextGroupId = 11,
        maxStations = 48,
        maxGroups = 24,
        maxBytes = 8192;
      const bytes = (s) => new TextEncoder().encode(s).length;
      const snapshot = () => JSON.stringify({ stations: rows, groups, hostname });
      const dirty = () => loaded && snapshot() !== original;
      const validName = (name) =>
        name && bytes(name) <= 63 && !/[\u0000-\u001f\u007f-\u009f]/u.test(name);
      function groupName(id) {
        const parts = [],
          seen = new Set();
        let current = groups.find((g) => g.id === id);
        while (current && !seen.has(current.id)) {
          seen.add(current.id);
          if (current.id === 0 && parts.length) break;
          parts.unshift(current.name);
          if (current.id === 0) break;
          current = groups.find((g) => g.id === current.parent);
        }
        return parts.join(" › ") || "Unbekannte Gruppe";
      }
      function allowedParent(id, parent) {
        const seen = new Set();
        while (true) {
          if (parent === id || seen.has(parent)) return false;
          if (parent === 0) return true;
          seen.add(parent);
          const group = groups.find((g) => g.id === parent);
          if (!group) return false;
          parent = group.parent;
        }
      }
      function message(text, error = false) {
        $("message").textContent = text;
        $("message").classList.toggle("error", error);
        if (text && error) $("message").scrollIntoView({ block: "nearest" });
      }
      function state() {
        const changed = dirty(),
          hostChanged = hostname !== activeHostname;
        $("save").disabled = !loaded || busy || restarting || !changed;
        $("reload").disabled = busy || restarting;
        $("workspace").disabled = busy || !loaded || restarting;
        $("add").disabled = rows.length >= maxStations;
        $("save").textContent = restarting
          ? "Neustart läuft …"
          : busy
            ? "Wird gespeichert …"
            : hostChanged
              ? "Speichern & neu starten"
              : "Änderungen speichern";
        $("save-state").replaceChildren();
        if (changed) {
          const dot = element("span", "changed");
          $("save-state").append(dot);
        }
        $("save-state").append(
          document.createTextNode(
            !loaded
              ? "Liste noch nicht geladen"
              : restarting
                ? "Gespeichert · Radio startet neu"
                : changed
                  ? "Ungespeicherte Änderungen"
                  : "Alles auf deinem Radio gespeichert",
          ),
        );
        $("save-hint").textContent = restarting
          ? "Öffne anschließend die neue Adresse oben."
          : hostChanged
            ? "Neuer Hostname: Neustart nach dem Speichern."
            : changed
              ? "Mit Speichern auf dein Radio übertragen"
              : "Bleibt auch nach dem Ausschalten erhalten";
        $("total").textContent = loaded ? rows.length : "–";
        $("hostname-display").textContent = hostname + ".local";
        $("group-total").textContent = groups.length + " Gruppen · Namen, Struktur und Reihenfolge";
      }
      function element(tag, className, text) {
        const node = document.createElement(tag);
        if (className) node.className = className;
        if (text !== undefined) node.textContent = text;
        return node;
      }
      function render(focusKey) {
        const query = $("search").value.trim().toLocaleLowerCase("de"),
          filter = $("filter").value;
        const visible = rows
          .map((s, i) => ({ s, i }))
          .filter(
            ({ s }) =>
              (!filter || s.group === Number(filter)) &&
              (!query || s.name.toLocaleLowerCase("de").includes(query)),
          );
        $("stations").replaceChildren();
        visible.forEach(({ s, i }, v) => {
          const li = element("li", "station");
          li.append(element("span", "number", String(i + 1).padStart(2, "0")));
          const info = element("div", "station-info");
          info.append(
            element("div", "station-name", s.name),
            element("span", "badge", groupName(s.group)),
          );
          const url = element("div", "url", s.url);
          url.title = s.url;
          info.append(url);
          li.append(info);
          const actions = element("div", "actions");
          [-1, 1].forEach((delta) => {
            const btn = element("button", "arrow", delta < 0 ? "↑" : "↓");
            btn.type = "button";
            btn.disabled = v + delta < 0 || v + delta >= visible.length;
            btn.setAttribute(
              "aria-label",
              s.name + (delta < 0 ? " nach oben verschieben" : " nach unten verschieben"),
            );
            btn.dataset.focus = i + ":" + delta;
            btn.onclick = () => {
              const target = visible[v + delta].i;
              [rows[i], rows[target]] = [rows[target], rows[i]];
              message("");
              render(target + ":" + delta);
            };
            actions.append(btn);
          });
          const edit = element("button", "edit", "Bearbeiten");
          edit.type = "button";
          edit.dataset.focus = i + ":edit";
          edit.setAttribute("aria-label", s.name + " bearbeiten");
          edit.onclick = () => openEditor(i);
          actions.append(edit);
          li.append(actions);
          $("stations").append(li);
        });
        $("empty").hidden = visible.length > 0;
        $("visible-count").textContent = visible.length + " von " + rows.length + " Sendern";
        state();
        if (focusKey) {
          const target = Array.from(document.querySelectorAll("[data-focus]")).find(
            (el) => el.dataset.focus === focusKey,
          );
          if (target && !target.disabled) target.focus();
          else if (target) target.parentElement.querySelector(".edit").focus();
        }
      }
      function ask(title, text, yesLabel = "Bestätigen") {
        return new Promise((resolve) => {
          const dialog = $("confirm");
          $("confirm-title").textContent = title;
          $("confirm-text").textContent = text;
          $("confirm-yes").textContent = yesLabel;
          const finish = (value) => {
            dialog.close();
            resolve(value);
          };
          $("confirm-yes").onclick = () => finish(true);
          $("confirm-no").onclick = () => finish(false);
          dialog.oncancel = (e) => {
            e.preventDefault();
            finish(false);
          };
          dialog.showModal();
          $("confirm-no").focus();
        });
      }
      function options() {
        for (const id of ["filter", "group"]) {
          const select = $(id),
            previous = select.value;
          select.replaceChildren();
          if (id === "filter") {
            const all = element("option", "", "Alle Gruppen");
            all.value = "";
            select.append(all);
          }
          for (const g of groups) {
            const opt = element("option", "", groupName(g.id));
            opt.value = g.id;
            select.append(opt);
          }
          if (Array.from(select.options).some((opt) => opt.value === previous))
            select.value = previous;
        }
      }
      async function request(url, options = {}) {
        const controller = new AbortController();
        const timer = setTimeout(() => controller.abort(), 15000);
        try {
          const response = await fetch(url, {
            cache: "no-store",
            ...options,
            signal: controller.signal,
          });
          const data = await response.json();
          if (!response.ok) throw new Error(data.error || "Anfrage fehlgeschlagen.");
          return data;
        } finally {
          clearTimeout(timer);
        }
      }
      function restartNotice(data) {
        message("Gespeichert. Das Radio startet neu. Anschließend öffnen: ");
        const link = element("a", "", "http://" + hostname + ".local/");
        link.id = "new-address";
        link.href = "http://" + hostname + ".local/";
        $("message").append(link);
        if (data.ipAddress && /^[0-9.]+$/.test(data.ipAddress)) {
          const ip = element("a", "", data.ipAddress);
          ip.href = "http://" + data.ipAddress + "/";
          $("message").append(document.createTextNode(" · Alternativ über die IP-Adresse: "), ip);
        }
        $("message").scrollIntoView({ block: "nearest" });
      }
      function adopt(data) {
        rows = data.stations;
        groups = data.groups;
        hostname = data.hostname || "webradio";
        activeHostname = data.activeHostname || hostname;
        restarting = Boolean(data.restarting);
        nextGroupId = data.nextGroupId ?? Math.max(...groups.map((g) => g.id)) + 1;
        token = data.token;
        revision = data.revision;
        maxStations = data.maxStations;
        maxGroups = data.maxGroups || 24;
        maxBytes = data.maxBytes;
        loaded = true;
        original = snapshot();
        $("connection").classList.add("ready");
        $("connection-text").textContent = restarting
          ? "Radio startet neu"
          : "Mit deinem Radio verbunden";
        options();
        render();
      }
      async function load() {
        if (
          dirty() &&
          !(await ask(
            "Änderungen verwerfen?",
            "Beim Neuladen gehen deine ungespeicherten Änderungen verloren.",
            "Neu laden",
          ))
        )
          return;
        busy = true;
        state();
        try {
          const data = await request("/api/stations");
          adopt(data);
          if (restarting) restartNotice(data);
          else message("");
        } catch (e) {
          message(
            "Liste konnte nicht geladen werden: " +
              e.message +
              " Prüfe die WLAN-Verbindung und versuche es erneut.",
            true,
          );
          $("connection").classList.remove("ready");
          $("connection-text").textContent = "Verbindung unterbrochen";
        } finally {
          busy = false;
          state();
        }
      }
      async function save() {
        if (!dirty() || busy || restarting) return;
        const body = JSON.stringify({ revision, stations: rows, groups, hostname });
        if (bytes(body) > maxBytes) {
          message(
            "Deine Radiokonfiguration ist größer als 8 KB. Bitte Namen oder URLs kürzen oder Einträge entfernen.",
            true,
          );
          return;
        }
        busy = true;
        state();
        try {
          const data = await request("/api/stations?revision=" + revision, {
            method: "POST",
            headers: { "Content-Type": "application/json", "X-Radio-Token": token },
            body,
          });
          adopt(data);
          if (restarting) restartNotice(data);
          else message("Gespeichert. Sender und Gruppen sind jetzt auf dem Radio verfügbar.");
        } catch (e) {
          message(
            "Speichern nicht bestätigt: " + e.message + " Deine Änderungen bleiben hier erhalten.",
            true,
          );
        } finally {
          busy = false;
          state();
        }
      }
      function openEditor(index) {
        editIndex = index;
        const s =
          index >= 0
            ? rows[index]
            : {
                name: "",
                url: "",
                group:
                  $("filter").value === ""
                    ? (groups.find((g) => g.id !== 0)?.id ?? 0)
                    : Number($("filter").value),
              };
        $("editor-title").textContent = index >= 0 ? "Sender bearbeiten" : "Neuer Lieblingssender";
        $("name").value = s.name;
        $("url").value = s.url;
        $("group").value = s.group;
        $("position").max = index >= 0 ? rows.length : rows.length + 1;
        $("position").value = index >= 0 ? index + 1 : rows.length + 1;
        $("delete").hidden = index < 0;
        $("delete").disabled = rows.length <= 1;
        $("delete").title = rows.length <= 1 ? "Mindestens ein Sender muss erhalten bleiben." : "";
        $("form-error").textContent = "";
        $("editor").showModal();
        $("name").focus();
      }
      $("station-form").onsubmit = (e) => {
        e.preventDefault();
        const name = $("name").value.trim(),
          url = $("url").value.trim(),
          group = Number($("group").value),
          position = Number($("position").value);
        let parsed;
        try {
          parsed = new URL(url);
        } catch {}
        if (!name || bytes(name) > 63 || /[\u0000-\u001f\u007f-\u009f]/u.test(name)) {
          $("form-error").textContent = "Bitte einen Namen mit maximal 63 UTF-8-Bytes eingeben.";
          return;
        }
        if (
          !parsed ||
          !["http:", "https:"].includes(parsed.protocol) ||
          !/^https?:\/\/[^/?#]/.test(url) ||
          bytes(url) > 255 ||
          /[^\x21-\x7e]|\\/.test(url)
        ) {
          $("form-error").textContent =
            "Bitte eine gültige HTTP(S)-URL mit maximal 255 Bytes eingeben. Sonderzeichen müssen URL-kodiert sein.";
          return;
        }
        if (!Number.isInteger(position) || position < 1 || position > Number($("position").max))
          return;
        const s = { id: editIndex >= 0 ? rows[editIndex].id : 0, name, url, group };
        if (editIndex >= 0) rows.splice(editIndex, 1);
        rows.splice(position - 1, 0, s);
        $("editor").close();
        message("");
        render(position - 1 + ":edit");
      };
      $("delete").onclick = async () => {
        if (rows.length <= 1) return;
        const index = editIndex;
        if (
          await ask(
            "Sender löschen?",
            "„" +
              rows[index].name +
              "“ wird aus deiner Liste entfernt. Wirksam wird das nach dem Speichern.",
            "Sender löschen",
          )
        ) {
          rows.splice(index, 1);
          $("editor").close();
          message("");
          render();
          $("add").focus();
        }
      };
      function renderGroups(focusId) {
        $("group-list").replaceChildren();
        groups.forEach((g, i) => {
          const item = element("li", "group-row");
          const info = element("div", "station-info");
          info.append(
            element("div", "station-name", g.name),
            element(
              "span",
              "badge",
              g.id === 0 ? "Startseite · bleibt erhalten" : groupName(g.parent),
            ),
          );
          const children = groups.filter((child) => child.id !== 0 && child.parent === g.id).length;
          info.append(
            element(
              "div",
              "dialog-intro",
              rows.filter((s) => s.group === g.id).length +
                " Sender · " +
                children +
                " Untergruppen",
            ),
          );
          item.append(info);
          const actions = element("div", "actions");
          for (const delta of [-1, 1]) {
            const button = element("button", "group-arrow", delta < 0 ? "↑" : "↓");
            button.type = "button";
            button.disabled = g.id === 0 || i + delta < 1 || i + delta >= groups.length;
            button.setAttribute("aria-label", g.name + (delta < 0 ? " nach oben" : " nach unten"));
            button.dataset.groupMove = g.id + ":" + delta;
            button.onclick = () => {
              [groups[i], groups[i + delta]] = [groups[i + delta], groups[i]];
              options();
              render();
              renderGroups(g.id + ":" + delta);
            };
            actions.append(button);
          }
          const edit = element("button", "group-edit", "Bearbeiten");
          edit.type = "button";
          edit.dataset.groupId = g.id;
          edit.setAttribute("aria-label", g.name + " bearbeiten");
          edit.onclick = () => openGroupEditor(g.id);
          actions.append(edit);
          item.append(actions);
          $("group-list").append(item);
        });
        $("group-add").disabled = groups.length >= maxGroups;
        $("groups-count").textContent =
          groups.length + " von " + maxGroups + " Gruppen einschließlich Startseite";
        if (focusId) {
          const button = Array.from(document.querySelectorAll("[data-group-move]")).find(
            (b) => b.dataset.groupMove === focusId,
          );
          if (button && !button.disabled) button.focus();
        }
      }
      function openGroupEditor(id) {
        editGroupId = id;
        const g = groups.find((g) => g.id === id);
        $("group-title").textContent = g ? "Gruppe bearbeiten" : "Neue Gruppe";
        $("group-name").value = g?.name || "";
        const parent = $("group-parent");
        parent.replaceChildren();
        parent.disabled = id === 0;
        for (const candidate of groups) {
          if (id !== 0 && id !== null && !allowedParent(id, candidate.id)) continue;
          if (id === 0 && candidate.id !== 0) continue;
          const option = element("option", "", groupName(candidate.id));
          option.value = candidate.id;
          parent.append(option);
        }
        parent.value = g?.parent ?? 0;
        $("group-position").disabled = id === 0;
        $("group-position").min = id === 0 ? 1 : 2;
        $("group-position").max = g ? groups.length : groups.length + 1;
        $("group-position").value = g ? groups.indexOf(g) + 1 : groups.length + 1;
        $("group-delete").hidden = !g || id === 0;
        $("group-error").textContent = "";
        $("group-editor").showModal();
        $("group-name").focus();
      }
      $("groups-open").onclick = () => {
        renderGroups();
        $("group-manager").showModal();
      };
      $("groups-close").onclick = $("groups-done").onclick = () => $("group-manager").close();
      $("group-add").onclick = () => openGroupEditor(null);
      $("group-close").onclick = $("group-cancel").onclick = () => $("group-editor").close();
      $("group-form").onsubmit = (e) => {
        e.preventDefault();
        const name = $("group-name").value.trim(),
          parent = Number($("group-parent").value),
          position = Number($("group-position").value);
        if (!validName(name)) {
          $("group-error").textContent =
            "Bitte einen Gruppennamen mit maximal 63 Zeichen eingeben; Umlaute zählen doppelt.";
          return;
        }
        if (editGroupId !== 0 && editGroupId !== null && !allowedParent(editGroupId, parent)) {
          $("group-error").textContent = "Diese Zuordnung würde eine Schleife bilden.";
          return;
        }
        if (
          !groups.some((g) => g.id === parent) ||
          !Number.isInteger(position) ||
          position < Number($("group-position").min) ||
          position > Number($("group-position").max)
        )
          return;
        if (editGroupId === null && (groups.length >= maxGroups || nextGroupId >= 4294967294))
          return;
        const group = {
          id: editGroupId ?? nextGroupId++,
          name,
          parent: editGroupId === 0 ? 0 : parent,
        };
        const index = groups.findIndex((g) => g.id === editGroupId);
        if (index >= 0) groups.splice(index, 1);
        groups.splice(position - 1, 0, group);
        $("group-editor").close();
        options();
        render();
        renderGroups();
      };
      $("group-delete").onclick = async () => {
        const group = groups.find((g) => g.id === editGroupId);
        if (!group || group.id === 0) return;
        const stations = rows.filter((s) => s.group === group.id).length,
          children = groups.filter((g) => g.parent === group.id).length;
        if (
          !(await ask(
            "Gruppe löschen?",
            "„" +
              group.name +
              "“ entfernen? " +
              stations +
              " Sender und " +
              children +
              " Untergruppen werden nach „" +
              groupName(group.parent) +
              "“ verschoben. Wirksam nach dem Speichern.",
            "Gruppe löschen",
          ))
        )
          return;
        rows.forEach((s) => {
          if (s.group === group.id) s.group = group.parent;
        });
        groups.forEach((g) => {
          if (g.parent === group.id) g.parent = group.parent;
        });
        groups = groups.filter((g) => g.id !== group.id);
        $("group-editor").close();
        options();
        render();
        renderGroups();
      };
      function hostnamePreview() {
        const value = $("hostname-input").value.trim().toLowerCase();
        $("hostname-preview").textContent = "http://" + (value || "dein-radio") + ".local/";
      }
      $("hostname-open").onclick = () => {
        $("hostname-input").value = hostname;
        $("hostname-error").textContent = "";
        hostnamePreview();
        $("hostname-editor").showModal();
        $("hostname-input").focus();
      };
      $("hostname-input").oninput = hostnamePreview;
      $("hostname-close").onclick = $("hostname-cancel").onclick = () =>
        $("hostname-editor").close();
      $("hostname-form").onsubmit = (e) => {
        e.preventDefault();
        const value = $("hostname-input").value.trim().toLowerCase();
        if (!/^[a-z0-9](?:[a-z0-9-]{0,29}[a-z0-9])?$/.test(value)) {
          $("hostname-error").textContent =
            "Bitte 1–31 Zeichen (a–z, 0–9, Bindestriche innerhalb des Namens) ohne .local eingeben.";
          return;
        }
        hostname = value;
        $("hostname-editor").close();
        state();
        message("");
      };
      $("close-editor").onclick = $("cancel-editor").onclick = () => $("editor").close();
      $("add").onclick = () => openEditor(-1);
      $("search").oninput = () => render();
      $("filter").onchange = () => render();
      $("reload").onclick = load;
      $("save").onclick = save;
      window.addEventListener("beforeunload", (e) => {
        if (dirty() || document.querySelector("dialog[open]")) {
          e.preventDefault();
          e.returnValue = "";
        }
      });
      load();
    </script>
  </body>
</html>
)WEBRADIO";
#endif
