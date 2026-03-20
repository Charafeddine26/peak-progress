/**
 * app.js — Peak Progress Companion Web App
 *
 * Web Bluetooth BLE integration + Demo Mode fallback.
 * Matches the Arduino BLE protocol from ble_service.h.
 *
 * Service UUID:  19B10000-E8F2-537E-4F6C-D104768A1214
 * Characteristics:
 *   0x0001 - Progress  (Read + Notify, 8 bytes)
 *   0x0002 - Mountain  (Read + Notify, 20 bytes string)
 *   0x0003 - Command   (Write, 1 byte)
 *   0x0004 - Unlock    (Read + Notify, 2 bytes bitfield)
 */

// ═══════════════════════════════════════════════════════
//  CONSTANTS
// ═══════════════════════════════════════════════════════

const SERVICE_UUID       = '19b10000-e8f2-537e-4f6c-d104768a1214';
const CHAR_PROGRESS_UUID = '19b10001-e8f2-537e-4f6c-d104768a1214';
const CHAR_MOUNTAIN_UUID = '19b10002-e8f2-537e-4f6c-d104768a1214';
const CHAR_COMMAND_UUID  = '19b10003-e8f2-537e-4f6c-d104768a1214';
const CHAR_UNLOCK_UUID   = '19b10004-e8f2-537e-4f6c-d104768a1214';

const CMD_LOG_ACTIVITY = 0x01;
const CMD_RESET        = 0x02;

// Mountain library (mirrors mountains.h in firmware)
const MOUNTAINS = [
  // TIER 1: Training Peaks
  { name: 'Colline Locale',       weeks: 1, sessions:  7, tier: 1, emoji: '01', unlockAfter: 0, gradient: ['#0000FF','#44FF00'] },
  { name: 'Petit Sommet',         weeks: 1, sessions:  7, tier: 1, emoji: '02', unlockAfter: 1, gradient: ['#0066FF','#00FF00'] },
  { name: "Mont d'Entraînement",  weeks: 1, sessions:  7, tier: 1, emoji: '03', unlockAfter: 2, gradient: ['#6600CC','#FFD700'] },
  // TIER 2: Regional Mountains
  { name: 'Mont Blanc',           weeks: 2, sessions: 14, tier: 2, emoji: '04', unlockAfter: 3, gradient: ['#00FFFF','#FFAA00'] },
  { name: 'Matterhorn',           weeks: 2, sessions: 14, tier: 2, emoji: '05', unlockAfter: 4, gradient: ['#000066','#FF0000'] },
  // TIER 3: Continental Peaks
  { name: 'Kilimanjaro',          weeks: 3, sessions: 21, tier: 3, emoji: '06', unlockAfter: 5, gradient: ['#FF6600','#FF4400'] },
  { name: 'Denali',               weeks: 3, sessions: 21, tier: 3, emoji: '07', unlockAfter: 6, gradient: ['#FFFFFF','#0044FF'] },
  // TIER 4: Ultimate Summits
  { name: 'Everest',              weeks: 4, sessions: 28, tier: 4, emoji: '08', unlockAfter: 7, gradient: ['#FF0000','#FFFFFF'] },
  { name: 'K2',                   weeks: 4, sessions: 28, tier: 4, emoji: '09', unlockAfter: 8, gradient: ['#330000','#FFFF00'] },
];

const TIER_NAMES = {
  1: 'Training Peaks',
  2: 'Regional Mountains',
  3: 'Continental Peaks',
  4: 'Ultimate Summits',
};

const TIER_COLORS = {
  1: '#00D2FF',
  2: '#6C63FF',
  3: '#FF9100',
  4: '#FF4081',
};

const ACHIEVEMENTS = [
  { name: 'First Step',      icon: '01', condition: s => s.totalSessions >= 1 },
  { name: 'First Summit',    icon: '02', condition: s => s.summits >= 1 },
  { name: 'Week Warrior',    icon: '03', condition: s => s.streak >= 7 },
  { name: 'Trail Blazer',    icon: '04', condition: s => s.summits >= 3 },
  { name: 'Alpine Pro',      icon: '05', condition: s => s.summits >= 5 },
  { name: 'Mountaineer',     icon: '06', condition: s => s.totalSessions >= 50 },
  { name: 'Summit Legend',   icon: '07', condition: s => s.summits >= 7 },
  { name: 'Everest Club',    icon: '08', condition: s => s.mountainIndex >= 7 },
  { name: 'Peak Master',     icon: '09', condition: s => s.summits >= 9 },
];

// LED palette colors per mountain (CSS colors for UI)
const LED_PALETTES = [
  ['#0000FF','#0022DD','#0044BB','#006699','#008877','#00AA55','#00CC33','#00EE11','#22FF00','#44FF00'],
  ['#0000FF','#0033FF','#0066FF','#0099FF','#00CCFF','#00FFCC','#00FF99','#00FF66','#00FF33','#00FF00'],
  ['#6600CC','#7711BB','#8822AA','#993399','#AA4488','#BB5577','#CC7744','#DD9922','#EEBB11','#FFD700'],
  ['#00FFFF','#33FFFF','#66FFFF','#99FFFF','#CCFFFF','#FFFFFF','#FFFFCC','#FFFF99','#FFDD44','#FFAA00'],
  ['#000066','#110077','#220088','#440099','#6600AA','#8800BB','#AA0099','#CC0066','#DD0033','#FF0000'],
  ['#FF6600','#FF7711','#FF8822','#FF9933','#FFAA44','#FFBB55','#FFCC44','#FFDD33','#FFAA22','#FF4400'],
  ['#FFFFFF','#DDEEFF','#BBDDFF','#99CCFF','#77BBFF','#55AAFF','#3399FF','#1188FF','#0077FF','#0044FF'],
  ['#FF0000','#FF7F00','#FFFF00','#00FF00','#0000FF','#4B0082','#9400D3','#FF00FF','#FF66FF','#FFFFFF'],
  ['#330000','#660000','#990000','#CC0000','#FF0000','#FF3300','#FF6600','#FF9900','#FFCC00','#FFFF00'],
];

// ═══════════════════════════════════════════════════════
//  STATE
// ═══════════════════════════════════════════════════════

let bleDevice = null;
let bleService = null;
let charProgress = null;
let charMountain = null;
let charCommand = null;
let charUnlock = null;
let isDemoMode = false;

let state = {
  mountainIndex: 0,
  sessionsOnMountain: 0,
  totalSessionsForMountain: 7,
  summits: 0,
  streak: 0,
  longestStreak: 0,
  totalSessions: 0,
  mountainName: 'Colline Locale',
  unlockedBitfield: 0x0001, // bit 0 = Colline Locale unlocked
};

// ═══════════════════════════════════════════════════════
//  BLE CONNECTION
// ═══════════════════════════════════════════════════════

async function connectBLE() {
  const statusDot = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');

  // Check for Web Bluetooth support
  if (!navigator.bluetooth) {
    if (window.location.protocol === 'http:' && window.location.hostname !== 'localhost' && window.location.hostname !== '127.0.0.1') {
      showToast('!', 'Web Bluetooth requires HTTPS or localhost. Please use a secure connection.');
      alert('Security Error: Web Bluetooth API is only available on HTTPS or localhost. You are currently on an insecure HTTP connection so the browser has blocked it.');
    } else {
      showToast('!', 'Web Bluetooth not supported here. Try Demo Mode.');
      alert('Web Bluetooth is not supported on this browser/OS combination, or is disabled. Use Chrome on Android/Windows/macOS.');
    }
    return;
  }

  try {
    statusDot.className = 'status-dot connecting';
    statusText.textContent = 'Scanning...';

    bleDevice = await navigator.bluetooth.requestDevice({
      filters: [{ services: [SERVICE_UUID] }],
    });

    statusText.textContent = 'Connecting...';

    bleDevice.addEventListener('gattserverdisconnected', onDisconnected);

    const server = await bleDevice.gatt.connect();
    bleService = await server.getPrimaryService(SERVICE_UUID);

    // Get characteristics (progress + command are required, others optional)
    charProgress = await bleService.getCharacteristic(CHAR_PROGRESS_UUID);
    charCommand  = await bleService.getCharacteristic(CHAR_COMMAND_UUID);

    try { charMountain = await bleService.getCharacteristic(CHAR_MOUNTAIN_UUID); } catch (e) { charMountain = null; }
    try { charUnlock   = await bleService.getCharacteristic(CHAR_UNLOCK_UUID); }   catch (e) { charUnlock = null; }

    // Subscribe to notifications
    await charProgress.startNotifications();
    charProgress.addEventListener('characteristicvaluechanged', onProgressChanged);

    if (charMountain) {
      await charMountain.startNotifications();
      charMountain.addEventListener('characteristicvaluechanged', onMountainChanged);
    }

    if (charUnlock) {
      await charUnlock.startNotifications();
      charUnlock.addEventListener('characteristicvaluechanged', onUnlockChanged);
    }

    // Initial read
    await readAllCharacteristics();

    // Connected!
    statusDot.className = 'status-dot connected';
    statusText.textContent = 'Connected to PeakProgress';
    isDemoMode = false;

    updateSettingsInfo('Connected', 'PeakProgress', 'BLE');

    // Switch to dashboard
    setTimeout(() => showScreen('screen-home'), 600);

  } catch (err) {
    console.error('BLE error:', err);
    statusDot.className = 'status-dot disconnected';
    statusText.textContent = 'Connection failed';
    showToast('!', 'Could not connect: ' + err.message);
  }
}

function onDisconnected() {
  const statusDot = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
  statusDot.className = 'status-dot disconnected';
  statusText.textContent = 'Disconnected';
  updateSettingsInfo('Disconnected', '—', '—');
  showToast('!', 'Device disconnected');
}

async function readAllCharacteristics() {
  try {
    // Read Progress (always available)
    const progVal = await charProgress.readValue();
    parseProgress(progVal);

    // Read Mountain name (optional — derive from progress if missing)
    if (charMountain) {
      const mtnVal = await charMountain.readValue();
      parseMountainName(mtnVal);
    } else {
      state.mountainName = (MOUNTAINS[state.mountainIndex] || MOUNTAINS[0]).name;
    }

    // Read Unlock bitfield (optional — derive from summits if missing)
    if (charUnlock) {
      const unlockVal = await charUnlock.readValue();
      parseUnlock(unlockVal);
    } else {
      deriveUnlockedFromProgress();
    }

    updateAllUI();
  } catch (err) {
    console.error('Read error:', err);
  }
}

// ─── BLE Notification Handlers ──────────────────────

function onProgressChanged(event) {
  parseProgress(event.target.value);
  // Derive missing data if firmware doesn't provide those characteristics
  if (!charMountain) {
    state.mountainName = (MOUNTAINS[state.mountainIndex] || MOUNTAINS[0]).name;
  }
  if (!charUnlock) {
    deriveUnlockedFromProgress();
  }
  updateAllUI();
}

function onMountainChanged(event) {
  parseMountainName(event.target.value);
  updateAllUI();
}

function onUnlockChanged(event) {
  parseUnlock(event.target.value);
  updateAllUI();
}

// ─── BLE Data Parsers ───────────────────────────────

function parseProgress(dataView) {
  state.mountainIndex              = dataView.getUint8(0);
  state.sessionsOnMountain         = dataView.getUint8(1);
  state.totalSessionsForMountain   = dataView.getUint8(2);
  state.summits                    = dataView.getUint8(3);
  state.streak                     = dataView.getUint8(4);
  state.longestStreak              = dataView.getUint8(5);
  state.totalSessions              = (dataView.getUint8(6) << 8) | dataView.getUint8(7);
}

function parseMountainName(dataView) {
  const decoder = new TextDecoder();
  let raw = decoder.decode(dataView);
  // Remove null terminator
  state.mountainName = raw.replace(/\0/g, '');
}

function parseUnlock(dataView) {
  state.unlockedBitfield = dataView.getUint8(0) | (dataView.getUint8(1) << 8);
}

function deriveUnlockedFromProgress() {
  let bitfield = 0;
  for (let i = 0; i < MOUNTAINS.length; i++) {
    if (state.summits >= MOUNTAINS[i].unlockAfter) {
      bitfield |= (1 << i);
    }
  }
  state.unlockedBitfield = bitfield;
}

// ─── BLE Write Commands ─────────────────────────────

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

  // Set initial demo state
  state = {
    mountainIndex: 0,
    sessionsOnMountain: 0,
    totalSessionsForMountain: 7,
    summits: 0,
    streak: 0,
    longestStreak: 0,
    totalSessions: 0,
    mountainName: 'Colline Locale',
    unlockedBitfield: 0x0001,
  };

  const statusDot = document.getElementById('statusDot');
  const statusText = document.getElementById('statusText');
  statusDot.className = 'status-dot connected';
  statusText.textContent = 'Demo Mode Active';

  updateSettingsInfo('Demo Mode', 'Simulated', 'Demo');

  updateAllUI();
  setTimeout(() => showScreen('screen-home'), 400);
}

function demoLogActivity() {
  const mtn = MOUNTAINS[state.mountainIndex];

  if (state.sessionsOnMountain >= mtn.sessions) return;

  state.sessionsOnMountain++;
  state.totalSessions++;
  state.streak++;
  if (state.streak > state.longestStreak) {
    state.longestStreak = state.streak;
  }

  // Check for summit
  if (state.sessionsOnMountain >= mtn.sessions) {
    state.summits++;

    // Unlock mountains based on summits count
    for (let i = 0; i < MOUNTAINS.length; i++) {
      if (state.summits >= MOUNTAINS[i].unlockAfter) {
        state.unlockedBitfield |= (1 << i);
      }
    }

    showToast('+', `Summit reached: ${mtn.name}!`);

    // Auto-advance to next mountain after a delay
    setTimeout(() => {
      const next = findNextMountain();
      state.mountainIndex = next;
      state.sessionsOnMountain = 0;
      state.mountainName = MOUNTAINS[next].name;
      state.totalSessionsForMountain = MOUNTAINS[next].sessions;
      updateAllUI();
      showToast('>', `New expedition: ${MOUNTAINS[next].name}`);
    }, 1500);
  }

  state.totalSessionsForMountain = mtn.sessions;
  updateAllUI();
}

function findNextMountain() {
  // Find the next unlocked mountain after the current one
  for (let i = state.mountainIndex + 1; i < MOUNTAINS.length; i++) {
    if (state.unlockedBitfield & (1 << i)) {
      return i;
    }
  }
  // If none found, stay on current (all mountains complete or loop)
  return state.mountainIndex;
}

function demoReset() {
  state = {
    mountainIndex: 0,
    sessionsOnMountain: 0,
    totalSessionsForMountain: 7,
    summits: 0,
    streak: 0,
    longestStreak: 0,
    totalSessions: 0,
    mountainName: 'Colline Locale',
    unlockedBitfield: 0x0001,
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

  // Re-enable after cooldown
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
  const mtn = MOUNTAINS[state.mountainIndex] || MOUNTAINS[0];
  const tierColor = TIER_COLORS[mtn.tier];
  const total = mtn.sessions;
  const current = state.sessionsOnMountain;
  const pct = total > 0 ? Math.min(current / total, 1) : 0;

  // Mountain hero
  document.getElementById('mountainName').textContent = mtn.name;
  document.getElementById('tierBadge').textContent = `TIER ${mtn.tier}`;
  document.getElementById('tierBadge').style.background = `${tierColor}22`;
  document.getElementById('tierBadge').style.color = tierColor;
  document.getElementById('tierBadge').style.borderColor = `${tierColor}44`;
  document.getElementById('mountainSubtitle').textContent =
    `${TIER_NAMES[mtn.tier]} • ${mtn.weeks} Week${mtn.weeks > 1 ? 's' : ''}`;

  // Hero background gradient
  const [g1, g2] = mtn.gradient;
  document.getElementById('mountainHeroBg').style.background =
    `linear-gradient(135deg, ${g1}33, ${g2}22)`;

  // Progress card
  document.getElementById('progressFraction').textContent = `${current} / ${total}`;
  document.getElementById('progressPercent').textContent = `${Math.round(pct * 100)}%`;
  document.getElementById('progressBar').style.width = `${pct * 100}%`;

  // Week label
  const weekNum = Math.floor(current / 7) + 1;
  document.getElementById('weekLabel').textContent =
    `Week ${Math.min(weekNum, mtn.weeks)} of ${mtn.weeks}`;

  // Climber track
  document.getElementById('trackFill').style.height = `${pct * 100}%`;
  document.getElementById('climberDot').style.bottom = `${pct * 100}%`;

  // LED dots
  renderLEDDots(state.mountainIndex, current, total);

  // Stats
  document.getElementById('statStreak').textContent = state.streak;
  document.getElementById('statSummits').textContent = state.summits;
  document.getElementById('statLongest').textContent = state.longestStreak;
  document.getElementById('statTotal').textContent = state.totalSessions;

  // Progress bar color
  const bar = document.getElementById('progressBar');
  bar.style.background = `linear-gradient(90deg, ${g1}, ${g2})`;
}

function renderLEDDots(mountainIdx, current, total) {
  const container = document.getElementById('ledDots');
  const palette = LED_PALETTES[mountainIdx] || LED_PALETTES[0];
  const litCount = total > 0 ? Math.floor((current / total) * 10) : 0;

  container.innerHTML = '';
  for (let i = 0; i < 10; i++) {
    const dot = document.createElement('div');
    dot.className = 'led-dot';
    if (i < litCount) {
      dot.classList.add('lit');
      dot.style.background = palette[i];
      dot.style.color = palette[i];
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

    const isUnlocked = !!(state.unlockedBitfield & (1 << idx));
    const isCurrent = idx === state.mountainIndex;
    const isCompleted = isUnlocked && idx < state.mountainIndex;
    const isLocked = !isUnlocked;

    const item = document.createElement('div');
    item.className = 'mountain-item';
    if (isCurrent) item.classList.add('current');
    if (isLocked) item.classList.add('locked');

    // Status
    let statusClass, statusText;
    if (isCurrent) {
      statusClass = 'in-progress';
      statusText = '▶ Active';
    } else if (isCompleted) {
      statusClass = 'completed';
      statusText = '✅ Done';
    } else if (isUnlocked) {
      statusClass = 'unlocked';
      statusText = '🔓 Ready';
    } else {
      statusClass = 'locked';
      statusText = `🔒 ${mtn.unlockAfter} summits`;
    }

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
        ${mtn.emoji}
      </div>
      <div class="mountain-item-info">
        <div class="mountain-item-name">${mtn.name}</div>
        <div class="mountain-item-detail">${mtn.weeks} week${mtn.weeks > 1 ? 's' : ''} • ${mtn.sessions} sessions</div>
        ${progressHTML}
      </div>
      <span class="mountain-item-status ${statusClass}">${statusText}</span>
    `;

    container.appendChild(item);
  });
}

// ─── History ────────────────────────────────────────

function updateHistory() {
  // Stats
  document.getElementById('histSummits').textContent = state.summits;
  document.getElementById('histTotal').textContent = state.totalSessions;
  document.getElementById('histStreak').textContent = state.streak;
  document.getElementById('histLongest').textContent = state.longestStreak;

  // Timeline
  renderTimeline();

  // Achievements
  renderAchievements();
}

function renderTimeline() {
  const container = document.getElementById('timeline');
  container.innerHTML = '';

  // Add completed mountains to timeline
  for (let i = 0; i <= state.mountainIndex && i < MOUNTAINS.length; i++) {
    const mtn = MOUNTAINS[i];
    const isCurrent = i === state.mountainIndex;

    const item = document.createElement('div');
    item.className = 'timeline-item';
    item.innerHTML = `
      <div class="timeline-dot ${isCurrent ? 'current' : ''}"></div>
      <div class="timeline-title">${mtn.emoji} ${mtn.name}</div>
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
        <div class="timeline-title">00 Journey begins</div>
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
  document.getElementById('settingsDevice').textContent = device;
  document.getElementById('settingsMode').textContent = mode;

  // Update header status badge
  const badge = document.getElementById('headerStatus');
  const dot = badge.querySelector('.status-dot-sm');
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
  // Deactivate all screens
  document.querySelectorAll('.screen').forEach(s => s.classList.remove('active'));

  // Activate target
  const target = document.getElementById(screenId);
  if (target) target.classList.add('active');

  // Update nav buttons across all screens
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
  // Generate LED dots for initial state
  renderLEDDots(0, 0, 7);
});
