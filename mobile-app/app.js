/**
 * app.js — Peak Progress Companion Web App (v2.0)
 *
 * Web Bluetooth BLE integration + Demo Mode fallback.
 * Matches v2.0 firmware with 2 BLE characteristics only.
 *
 * Service UUID:  19B10000-E8F2-537E-4F6C-D104768A1214
 * Characteristics:
 *   0x0001 - Progress  (Read + Notify, 8 bytes)
 *   0x0003 - Command   (Write, 1 byte)
 */

// ═══════════════════════════════════════════════════════
//  CONSTANTS
// ═══════════════════════════════════════════════════════

const SERVICE_UUID       = '19b10000-e8f2-537e-4f6c-d104768a1214';
const CHAR_PROGRESS_UUID = '19b10001-e8f2-537e-4f6c-d104768a1214';
const CHAR_COMMAND_UUID  = '19b10003-e8f2-537e-4f6c-d104768a1214';

const CMD_LOG_ACTIVITY = 0x01;
const CMD_RESET        = 0x02;

// Mountain library (mirrors firmware exactly)
const MOUNTAINS = [
  { name: 'Colline Locale',    sessions: 3, tier: 1, unlockAfter: 0 },
  { name: 'Petit Sommet',      sessions: 3, tier: 1, unlockAfter: 1 },
  { name: 'Mont Entrainement', sessions: 3, tier: 1, unlockAfter: 2 },
];

const TIER_NAMES  = { 1: 'Training Peaks' };
const TIER_COLORS = { 1: '#00D2FF' };

const ACHIEVEMENTS = [
  { name: 'First Step',   icon: '01', condition: s => s.totalSessions >= 1 },
  { name: 'First Summit', icon: '02', condition: s => s.summits >= 1 },
  { name: 'Week Warrior', icon: '03', condition: s => s.streak >= 7 },
  { name: 'Trail Blazer', icon: '04', condition: s => s.summits >= 3 },
  { name: 'Peak Master',  icon: '05', condition: s => s.totalSessions >= 9 },
];

// ═══════════════════════════════════════════════════════
//  STATE
// ═══════════════════════════════════════════════════════

let bleDevice   = null;
let bleService  = null;
let charProgress = null;
let charCommand  = null;
let isDemoMode   = false;

let state = {
  mountainIndex:      0,
  sessionsOnMountain: 0,
  sessionsNeeded:     3,
  summits:            0,
  streak:             0,
  longestStreak:      0,
  totalSessions:      0,
};

// ═══════════════════════════════════════════════════════
//  BLE CONNECTION
// ═══════════════════════════════════════════════════════

async function connectBLE() {
  const statusDot  = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');

  if (!navigator.bluetooth) {
    if (window.location.protocol === 'http:' &&
        window.location.hostname !== 'localhost' &&
        window.location.hostname !== '127.0.0.1') {
      showToast('!', 'Web Bluetooth requires HTTPS or localhost.');
      alert('Security Error: Web Bluetooth API is only available on HTTPS or localhost.');
    } else {
      showToast('!', 'Web Bluetooth not supported here. Try Demo Mode.');
      alert('Web Bluetooth is not supported on this browser/OS. Use Chrome on Android/Windows/macOS.');
    }
    return;
  }

  try {
    statusDot.className = 'status-dot connecting';
    statusText.textContent = 'Scanning...';

    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ services: [SERVICE_UUID] }],
    });

    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);

    // Connect with retry — handles stale GATT cache transparently
    const MAX_RETRIES = 3;
    for (let attempt = 1; attempt <= MAX_RETRIES; attempt++) {
      statusText.textContent = attempt === 1 ? 'Connecting...' : `Reconnecting (${attempt}/${MAX_RETRIES})...`;

      try {
        const server = await bleDevice.gatt.connect();
        bleService = await server.getPrimaryService(SERVICE_UUID);
        charProgress = await bleService.getCharacteristic(CHAR_PROGRESS_UUID);
        charCommand  = await bleService.getCharacteristic(CHAR_COMMAND_UUID);
        break; // success
      } catch (charErr) {
        // If characteristic not found, disconnect and retry (clears GATT cache)
        if (attempt < MAX_RETRIES) {
          console.warn(`BLE attempt ${attempt} failed (${charErr.message}), retrying...`);
          if (bleDevice.gatt.connected) bleDevice.gatt.disconnect();
          await new Promise(r => setTimeout(r, 1000));
        } else {
          throw charErr;
        }
      }
    }

    // Subscribe to Progress notifications
    await charProgress.startNotifications();
    charProgress.addEventListener('characteristicvaluechanged', onProgressChanged);

    // Initial read
    const progVal = await charProgress.readValue();
    parseProgress(progVal);
    updateAllUI();

    // Connected
    statusDot.className = 'status-dot connected';
    statusText.textContent = 'Connected to PeakProgress';
    isDemoMode = false;

    updateSettingsInfo('Connected', 'PeakProgress', 'BLE');
    setTimeout(() => showScreen('screen-home'), 600);

  } catch (err) {
    console.error('BLE error:', err);
    statusDot.className = 'status-dot disconnected';
    statusText.textContent = 'Connection failed';
    showToast('!', 'Could not connect: ' + err.message);
  }
}

function onDisconnected() {
  const statusDot  = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
  statusDot.className = 'status-dot disconnected';
  statusText.textContent = 'Disconnected';
  updateSettingsInfo('Disconnected', '—', '—');
  showToast('!', 'Device disconnected');
}

// ─── BLE Notification Handler ─────────────────────────

function onProgressChanged(event) {
  parseProgress(event.target.value);
  updateAllUI();
}

// ─── BLE Data Parser ──────────────────────────────────

function parseProgress(dataView) {
  state.mountainIndex      = dataView.getUint8(0);
  state.sessionsOnMountain = dataView.getUint8(1);
  state.sessionsNeeded     = dataView.getUint8(2);
  state.summits            = dataView.getUint8(3);
  state.streak             = dataView.getUint8(4);
  state.longestStreak      = dataView.getUint8(5);
  state.totalSessions      = (dataView.getUint8(6) << 8) | dataView.getUint8(7);
}

// ─── BLE Write Command ───────────────────────────────

async function sendBLECommand(cmd) {
  if (charCommand) {
    try {
      await charCommand.writeValue(new Uint8Array([cmd]));
    } catch (err) {
      console.error('Write error:', err);
      showToast('!', 'Failed to send command');
    }
  }
}

// ═══════════════════════════════════════════════════════
//  DEMO MODE
// ═══════════════════════════════════════════════════════

function startDemoMode() {
  isDemoMode = true;

  state = {
    mountainIndex:      0,
    sessionsOnMountain: 0,
    sessionsNeeded:     MOUNTAINS[0].sessions,
    summits:            0,
    streak:             0,
    longestStreak:      0,
    totalSessions:      0,
  };

  const statusDot  = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
  statusDot.className = 'status-dot connected';
  statusText.textContent = 'Demo Mode Active';

  updateSettingsInfo('Demo Mode', 'Simulated', 'Demo');
  updateAllUI();
  setTimeout(() => showScreen('screen-home'), 400);
}

function demoLogActivity() {
  const mtn = MOUNTAINS[state.mountainIndex];

  state.sessionsOnMountain++;
  state.totalSessions++;
  state.streak++;
  if (state.streak > state.longestStreak) {
    state.longestStreak = state.streak;
  }

  // Check for summit
  if (state.sessionsOnMountain >= mtn.sessions) {
    state.summits++;
    showToast('+', `Summit reached: ${mtn.name}!`);

    // Advance to next mountain if available and unlocked
    const next = state.mountainIndex + 1;
    if (next < MOUNTAINS.length && state.summits >= MOUNTAINS[next].unlockAfter) {
      state.mountainIndex = next;
      state.sessionsOnMountain = 0;
      state.sessionsNeeded = MOUNTAINS[next].sessions;
    }
  }

  updateAllUI();
}

function demoReset() {
  state = {
    mountainIndex:      0,
    sessionsOnMountain: 0,
    sessionsNeeded:     MOUNTAINS[0].sessions,
    summits:            0,
    streak:             0,
    longestStreak:      0,
    totalSessions:      0,
  };
  updateAllUI();
  showToast('<', 'Progress reset. Back to Colline Locale.');
}

// ═══════════════════════════════════════════════════════
//  USER ACTIONS
// ═══════════════════════════════════════════════════════

async function sendLogActivity() {
  const btn = document.getElementById('btnLog');
  btn.disabled = true;

  if (isDemoMode) {
    demoLogActivity();
    showToast('+', 'Activity logged! +1 session');
  } else {
    await sendBLECommand(CMD_LOG_ACTIVITY);
    showToast('+', 'Activity logged!');
  }

  setTimeout(() => { btn.disabled = false; }, 1000);
}

function confirmReset() {
  document.getElementById('resetModal').classList.add('active');
}

function closeModal() {
  document.getElementById('resetModal').classList.remove('active');
}

async function executeReset() {
  closeModal();
  if (isDemoMode) {
    demoReset();
  } else {
    await sendBLECommand(CMD_RESET);
    showToast('<', 'Reset command sent');
  }
  showScreen('screen-home');
}

// ═══════════════════════════════════════════════════════
//  UI UPDATE
// ═══════════════════════════════════════════════════════

function updateAllUI() {
  updateDashboard();
  renderMountainList();
  updateHistory();
}

function updateDashboard() {
  const mtn       = MOUNTAINS[state.mountainIndex] || MOUNTAINS[0];
  const tierColor = TIER_COLORS[mtn.tier];
  const current   = state.sessionsOnMountain;
  const total     = state.sessionsNeeded;
  const pct       = total > 0 ? Math.min(current / total, 1) : 0;

  // Mountain hero
  document.getElementById('mountainName').textContent = mtn.name;

  const tierBadge = document.getElementById('tierBadge');
  tierBadge.textContent = 'TIER 1';
  tierBadge.style.background  = `${tierColor}22`;
  tierBadge.style.color       = tierColor;
  tierBadge.style.borderColor = `${tierColor}44`;

  document.getElementById('mountainSubtitle').textContent =
    `${TIER_NAMES[mtn.tier]} \u2022 ${mtn.sessions} Sessions`;

  // Hero background gradient
  document.getElementById('mountainHeroBg').style.background =
    `linear-gradient(135deg, ${tierColor}33, ${tierColor}22)`;

  // Progress card
  document.getElementById('progressFraction').textContent = `${current} / ${total}`;
  document.getElementById('progressPercent').textContent = `${Math.round(pct * 100)}%`;

  const bar = document.getElementById('progressBar');
  bar.style.width      = `${pct * 100}%`;
  bar.style.background = tierColor;

  // Session label
  document.getElementById('weekLabel').textContent =
    `Session ${current} of ${total}`;

  // Climber track
  document.getElementById('trackFill').style.height  = `${pct * 100}%`;
  document.getElementById('climberDot').style.bottom  = `${pct * 100}%`;

  // LED dots
  renderLEDDots(state.mountainIndex, current, total);

  // Stats
  document.getElementById('statStreak').textContent  = state.streak;
  document.getElementById('statSummits').textContent  = state.summits;
  document.getElementById('statLongest').textContent  = state.longestStreak;
  document.getElementById('statTotal').textContent    = state.totalSessions;
}

function renderLEDDots(mountainIdx, current, total) {
  const container = document.getElementById('ledDots');
  const mtn       = MOUNTAINS[mountainIdx] || MOUNTAINS[0];
  const tierColor = TIER_COLORS[mtn.tier];
  const litCount  = total > 0 ? Math.floor((current / total) * 10) : 0;

  container.innerHTML = '';
  for (let i = 0; i < 10; i++) {
    const dot = document.createElement('div');
    dot.className = 'led-dot';
    if (i < litCount) {
      dot.classList.add('lit');
      dot.style.background = tierColor;
      dot.style.color      = tierColor;
    }
    container.appendChild(dot);
  }
}

// ─── Mountain Library ───────────────────────────────

function renderMountainList() {
  const container = document.getElementById('mountainList');
  container.innerHTML = '';

  let currentTier = 0;

  MOUNTAINS.forEach((mtn, idx) => {
    // Tier header
    if (mtn.tier !== currentTier) {
      currentTier = mtn.tier;
      const header = document.createElement('div');
      header.className = 'tier-header';
      header.innerHTML = `
        <div class="tier-header-dot" style="background:${TIER_COLORS[mtn.tier]}"></div>
        <span class="tier-header-text">Tier ${mtn.tier}: ${TIER_NAMES[mtn.tier]}</span>
      `;
      container.appendChild(header);
    }

    const isUnlocked  = state.summits >= mtn.unlockAfter;
    const isCurrent   = idx === state.mountainIndex;
    const isCompleted = isUnlocked && idx < state.mountainIndex;
    const isLocked    = !isUnlocked;

    const item = document.createElement('div');
    item.className = 'mountain-item';
    if (isCurrent) item.classList.add('current');
    if (isLocked)  item.classList.add('locked');

    // Status
    let statusClass, statusText;
    if (isCurrent) {
      statusClass = 'in-progress';
      statusText  = 'Active';
    } else if (isCompleted) {
      statusClass = 'completed';
      statusText  = 'Done';
    } else if (isUnlocked) {
      statusClass = 'unlocked';
      statusText  = 'Ready';
    } else {
      statusClass = 'locked';
      statusText  = `${mtn.unlockAfter} summits`;
    }

    // Mini progress bar for current mountain
    let progressHTML = '';
    if (isCurrent) {
      const pct = mtn.sessions > 0
        ? Math.min(state.sessionsOnMountain / mtn.sessions, 1) * 100
        : 0;
      progressHTML = `
        <div class="mountain-mini-progress">
          <div class="mountain-mini-progress-fill" style="width:${pct}%"></div>
        </div>
      `;
    }

    item.innerHTML = `
      <div class="mountain-item-icon" style="background:${TIER_COLORS[mtn.tier]}18; color:${TIER_COLORS[mtn.tier]}">
        T${mtn.tier}
      </div>
      <div class="mountain-item-info">
        <div class="mountain-item-name">${mtn.name}</div>
        <div class="mountain-item-detail">${mtn.sessions} sessions</div>
        ${progressHTML}
      </div>
      <span class="mountain-item-status ${statusClass}">${statusText}</span>
    `;

    container.appendChild(item);
  });
}

// ─── History ────────────────────────────────────────

function updateHistory() {
  document.getElementById('histSummits').textContent = state.summits;
  document.getElementById('histTotal').textContent   = state.totalSessions;
  document.getElementById('histStreak').textContent  = state.streak;
  document.getElementById('histLongest').textContent = state.longestStreak;

  renderTimeline();
  renderAchievements();
}

function renderTimeline() {
  const container = document.getElementById('timeline');
  container.innerHTML = '';

  // Show completed + current mountains
  for (let i = 0; i <= state.mountainIndex && i < MOUNTAINS.length; i++) {
    const mtn       = MOUNTAINS[i];
    const isCurrent = i === state.mountainIndex;

    const item = document.createElement('div');
    item.className = 'timeline-item';
    item.innerHTML = `
      <div class="timeline-dot ${isCurrent ? 'current' : ''}"></div>
      <div class="timeline-title">T${mtn.tier} ${mtn.name}</div>
      <div class="timeline-detail">${
        isCurrent
          ? `In progress — ${state.sessionsOnMountain}/${mtn.sessions} sessions`
          : `Completed — ${mtn.sessions} sessions`
      }</div>
    `;
    container.appendChild(item);
  }

  if (state.mountainIndex === 0 && state.sessionsOnMountain === 0) {
    container.innerHTML = `
      <div class="timeline-item">
        <div class="timeline-dot current"></div>
        <div class="timeline-title">Journey begins</div>
        <div class="timeline-detail">Log your first activity to start climbing</div>
      </div>
    `;
  }
}

function renderAchievements() {
  const container = document.getElementById('achievementsGrid');
  container.innerHTML = '';

  ACHIEVEMENTS.forEach(a => {
    const earned = a.condition(state);
    const div = document.createElement('div');
    div.className = `achievement ${earned ? 'earned' : 'locked'}`;
    div.innerHTML = `
      <div class="achievement-icon">${earned ? a.icon : '-'}</div>
      <div class="achievement-name">${a.name}</div>
    `;
    container.appendChild(div);
  });
}

// ─── Settings ───────────────────────────────────────

function updateSettingsInfo(connection, device, mode) {
  document.getElementById('settingsConnection').textContent = connection;
  document.getElementById('settingsDevice').textContent     = device;
  document.getElementById('settingsMode').textContent       = mode;

  const badge = document.getElementById('headerStatus');
  const dot   = badge.querySelector('.status-dot-sm');
  if (connection === 'Connected' || connection === 'Demo Mode') {
    dot.className = 'status-dot-sm connected';
    badge.querySelector('span').textContent = connection;
  } else {
    dot.className = 'status-dot-sm disconnected';
    badge.querySelector('span').textContent = 'Disconnected';
  }
}

// ═══════════════════════════════════════════════════════
//  NAVIGATION
// ═══════════════════════════════════════════════════════

function showScreen(screenId) {
  document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));

  const target = document.getElementById(screenId);
  if (target) target.classList.add('active');

  document.querySelectorAll('.nav-item').forEach(btn => {
    btn.classList.toggle('active', btn.dataset.screen === screenId);
  });
}

// ═══════════════════════════════════════════════════════
//  TOAST
// ═══════════════════════════════════════════════════════

let toastTimer = null;

function showToast(icon, text) {
  const toast = document.getElementById('toast');
  document.getElementById('toastIcon').textContent = icon;
  document.getElementById('toastText').textContent = text;

  toast.classList.add('show');

  if (toastTimer) clearTimeout(toastTimer);
  toastTimer = setTimeout(() => {
    toast.classList.remove('show');
  }, 2500);
}

// ═══════════════════════════════════════════════════════
//  INIT
// ═══════════════════════════════════════════════════════

document.addEventListener('DOMContentLoaded', () => {
  renderLEDDots(0, 0, MOUNTAINS[0].sessions);
});
