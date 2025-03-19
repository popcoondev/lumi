// グローバル変数
let connectionStatus = 'connecting'; // 'connecting', 'connected', 'disconnected'
let patterns = [];
let currentColorHex = '#ffff00'; // デフォルト色（黄色）

// JSONパターンのテンプレート
const jsonPatternTemplate = `{
  "name": "カスタムパターン",
  "type": "custom",
  "parameters": {
    "loop": true,
    "stepDelay": 100,
    "colorHSV": {
      "h": 0,
      "s": 255,
      "v": 255
    },
    "effects": {
      "fade": {
        "enabled": false
      },
      "blur": {
        "enabled": false
      }
    }
  },
  "steps": [
    {
      "faceSelection": {
        "mode": "all"
      },
      "colorHSV": {
        "h": 0,
        "s": 255,
        "v": 255
      },
      "duration": 1000
    },
    {
      "faceSelection": {
        "mode": "all"
      },
      "colorHSV": {
        "h": 120,
        "s": 255,
        "v": 255
      },
      "duration": 1000
    }
  ]
}`;

// DOMが読み込まれたら実行
document.addEventListener('DOMContentLoaded', () => {
    // モーダル用のスタイルを追加
    addModalStyles();
    
    // LEDControllerを初期化
    window.ledController = new LEDController();
    
    // APIステータスの確認
    checkApiStatus();
    
    // パターンリストの取得
    fetchPatterns();
    
    // JSONパターンボタンのイベントリスナーを追加
    document.getElementById('jsonPatternBtn').addEventListener('click', showJsonPatternModal);
});

// LEDControllerクラス - オクタゴンUIの制御
class LEDController {
    constructor() {
        this.canvas = document.getElementById('ledController');
        this.ctx = this.canvas.getContext('2d');
        this.ledStates = new Array(8).fill(false);
        this.ledColors = new Array(8).fill(currentColorHex); // デフォルト色（黄色）
        this.centerPressed = false;
        this.isSliding = false;
        this.touchedSegments = new Set();
        this.currentTouchState = false;
        this.focusedSegment = -1; // フォーカスされているセグメント（-1は未フォーカス）

        this.setupCanvas();
        this.setupButtons();
        this.addEventListeners();
        this.draw();
    }

    setupCanvas() {
        const container = this.canvas.parentElement;
        const size = Math.min(container.offsetWidth, container.offsetHeight);
        this.canvas.width = size;
        this.canvas.height = size;

        this.center = size / 2;
        this.radius = (size / 2) * 0.8; // 外側の八角形
        this.innerRadius = this.radius * 0.6; // 内側の八角形
        this.focusRadius = this.radius * 1.1; // フォーカスライン用の半径
    }

    setupButtons() {
        document.getElementById('resetBtn').addEventListener('click', () => {
            this.setAllLEDs(false);
            this.focusedSegment = -1;
            this.draw();
        });

        document.getElementById('patternBtn').addEventListener('click', () => {
            // パターンモーダルを表示
            showPatternModal();
        });
        
        // 色選択ボタンを追加
        const colorBtn = document.createElement('button');
        colorBtn.id = 'colorBtn';
        colorBtn.className = 'btn btn-outline-success w-100';
        colorBtn.textContent = 'Color';
        
        // ボタンを追加する場所を特定
        const buttonRow = document.getElementById('resetBtn').parentElement.parentElement;
        const newCol = document.createElement('div');
        newCol.className = 'col';
        newCol.appendChild(colorBtn);
        buttonRow.appendChild(newCol);
        
        // 色選択ボタンのイベント
        colorBtn.addEventListener('click', () => {
            // カラーピッカーモーダルを表示
            showColorPickerModal();
        });
    }

    addEventListeners() {
        this.canvas.addEventListener('pointerdown', this.handlePointerDown.bind(this));
        this.canvas.addEventListener('pointermove', this.handlePointerMove.bind(this));
        this.canvas.addEventListener('pointerup', this.handlePointerUp.bind(this));
        this.canvas.addEventListener('pointerout', this.handlePointerUp.bind(this));
        window.addEventListener('resize', () => {
            this.setupCanvas();
            this.draw();
        });
    }

    getOctagonPoints(radius) {
        const points = [];
        for (let i = 0; i < 8; i++) {
            // -π/8（-22.5度）回転させて、底辺をx軸と平行にする
            const angle = (i * Math.PI / 4) - (Math.PI / 8);
            points.push({
                x: this.center + radius * Math.cos(angle),
                y: this.center + radius * Math.sin(angle)
            });
        }
        return points;
    }

    getLEDSegment(x, y) {
        const distance = Math.sqrt(
            Math.pow(x - this.center, 2) + Math.pow(y - this.center, 2)
        );

        // 内側の八角形の中にある場合は中央ボタン
        if (distance <= this.innerRadius) {
            return -1;
        }

        // 外側の八角形の外にある場合は無効
        if (distance > this.radius) {
            return -2;
        }

        // 角度を計算（-22.5度から時計回りに）
        let angle = Math.atan2(y - this.center, x - this.center);
        // 角度を[0, 2π]の範囲に正規化
        if (angle < 0) angle += 2 * Math.PI;
        // -22.5度（-π/8）の回転を考慮して角度を調整
        angle = (angle + Math.PI / 8) % (2 * Math.PI);

        // 8つのセグメントに分割（0から7）
        return Math.floor(angle / (Math.PI / 4));
    }

    async handlePointerDown(e) {
        const rect = this.canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;

        const segment = this.getLEDSegment(x, y);

        if (segment === -1) {
            // 内側の八角形内（中央ボタン）
            this.centerPressed = true;
            if (this.focusedSegment !== -1) {
                // フォーカスされている面の状態を切り替え
                await this.toggleLED(this.focusedSegment);
            } else {
                // フォーカスがない場合は従来通り全体を切り替え
                const newState = !this.ledStates.every(state => state);
                await this.setAllLEDs(newState);
            }
        } else if (segment >= 0) {
            // LED セグメント
            this.focusedSegment = segment; // タップした面をフォーカス
            this.isSliding = true;
            this.touchedSegments.clear();
            this.currentTouchState = !this.ledStates[segment];
            await this.toggleLED(segment);
            this.touchedSegments.add(segment);
        }

        this.draw();
    }

    async handlePointerMove(e) {
        if (!this.isSliding) return;

        const rect = this.canvas.getBoundingClientRect();
        const x = e.clientX - rect.left;
        const y = e.clientY - rect.top;

        const segment = this.getLEDSegment(x, y);

        if (segment >= 0 && !this.touchedSegments.has(segment)) {
            await this.setLED(segment, this.currentTouchState);
            this.touchedSegments.add(segment);
        }

        this.draw();
    }

    handlePointerUp() {
        this.centerPressed = false;
        this.isSliding = false;
        this.touchedSegments.clear();
        this.draw();
    }

    async setAllLEDs(state) {
        try {
            // グローバル関数を呼び出し
            if (state) {
                // 点灯の場合は各面に対して個別に設定
                const r = parseInt(currentColorHex.substring(1, 3), 16);
                const g = parseInt(currentColorHex.substring(3, 5), 16);
                const b = parseInt(currentColorHex.substring(5, 7), 16);
                
                const promises = [];
                for (let i = 0; i < 8; i++) {
                    promises.push(
                        fetch(`/api/led/face/${i}?r=${r}&g=${g}&b=${b}`, {
                            method: 'POST'
                        })
                    );
                }
                await Promise.all(promises);
            } else {
                // 消灯の場合はリセットAPIを使用
                await fetch('/api/led/reset', {
                    method: 'POST'
                });
            }
            
            // UIの更新
            this.ledStates.fill(state);
            if (state) {
                // 点灯時は現在の色を設定
                for (let i = 0; i < 8; i++) {
                    this.ledColors[i] = currentColorHex;
                }
            }
            this.showStatus('すべてのLEDを' + (state ? '点灯' : '消灯') + 'しました', 'success');
        } catch (error) {
            console.error('Error setting all LEDs:', error);
            this.showStatus(error.message || 'LEDの設定に失敗しました', 'danger');
            
            // デモ用に状態を更新
            this.ledStates.fill(state);
            if (state) {
                for (let i = 0; i < 8; i++) {
                    this.ledColors[i] = currentColorHex;
                }
            }
        }
    }

    async setLED(segment, state) {
        try {
            if (state) {
                // 色を16進数から RGB に変換
                const r = parseInt(currentColorHex.substring(1, 3), 16);
                const g = parseInt(currentColorHex.substring(3, 5), 16);
                const b = parseInt(currentColorHex.substring(5, 7), 16);
                
                await fetch(`/api/led/face/${segment}?r=${r}&g=${g}&b=${b}`, {
                    method: 'POST'
                });
            } else {
                // 個別の消灯APIがないため、点灯時のみAPIを呼び出す
                // 消灯時はUIのみ更新
            }
            
            // UIの更新
            this.ledStates[segment] = state;
            if (state) {
                this.ledColors[segment] = currentColorHex;
            }
            this.showStatus(`LED ${segment + 1}を${state ? '点灯' : '消灯'}しました`, 'success');
        } catch (error) {
            console.error('Error setting LED:', error);
            this.showStatus(error.message || '通信エラーが発生しました', 'danger');
            
            // デモ用に状態を更新
            this.ledStates[segment] = state;
            if (state) {
                this.ledColors[segment] = currentColorHex;
            }
        }
    }

    async toggleLED(segment) {
        const newState = !this.ledStates[segment];
        await this.setLED(segment, newState);
    }

    showStatus(message, type) {
        // グローバル関数を呼び出し
        showStatus(message, type);
    }

    drawFocusLine(segment) {
        const startAngle = (segment * Math.PI / 4) - (Math.PI / 8);
        const endAngle = ((segment + 1) * Math.PI / 4) - (Math.PI / 8);
        const midAngle = (startAngle + endAngle) / 2;

        // フォーカスラインの長さ
        const lineLength = this.radius * 0.2;

        // フォーカスラインの始点と終点を計算
        const startX = this.center + this.focusRadius * Math.cos(midAngle);
        const startY = this.center + this.focusRadius * Math.sin(midAngle);
        const endX = this.center + (this.focusRadius + lineLength) * Math.cos(midAngle);
        const endY = this.center + (this.focusRadius + lineLength) * Math.sin(midAngle);

        // フォーカスラインを描画
        this.ctx.beginPath();
        this.ctx.moveTo(startX, startY);
        this.ctx.lineTo(endX, endY);
        this.ctx.strokeStyle = '#fff';
        this.ctx.lineWidth = 3;
        this.ctx.stroke();
        this.ctx.lineWidth = 1;
    }

    draw() {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);

        const outerPoints = this.getOctagonPoints(this.radius);
        const innerPoints = this.getOctagonPoints(this.innerRadius);

        // Draw LED segments between outer and inner octagons
        for (let i = 0; i < 8; i++) {
            this.ctx.beginPath();
            // 外側の八角形の2点
            this.ctx.moveTo(outerPoints[i].x, outerPoints[i].y);
            this.ctx.lineTo(outerPoints[(i + 1) % 8].x, outerPoints[(i + 1) % 8].y);
            // 内側の八角形の2点
            this.ctx.lineTo(innerPoints[(i + 1) % 8].x, innerPoints[(i + 1) % 8].y);
            this.ctx.lineTo(innerPoints[i].x, innerPoints[i].y);
            this.ctx.closePath();

            // 点灯状態に応じた色を設定
            if (this.ledStates[i]) {
                // 16進数の色をRGBAに変換
                const r = parseInt(this.ledColors[i].substring(1, 3), 16);
                const g = parseInt(this.ledColors[i].substring(3, 5), 16);
                const b = parseInt(this.ledColors[i].substring(5, 7), 16);
                this.ctx.fillStyle = `rgba(${r}, ${g}, ${b}, 0.5)`;
            } else {
                this.ctx.fillStyle = 'rgba(128, 128, 128, 0.3)';
            }
            
            this.ctx.fill();
            this.ctx.strokeStyle = '#fff';
            this.ctx.stroke();
        }

        // Draw inner octagon (center button)
        this.ctx.beginPath();
        innerPoints.forEach((point, i) => {
            if (i === 0) {
                this.ctx.moveTo(point.x, point.y);
            } else {
                this.ctx.lineTo(point.x, point.y);
            }
        });
        this.ctx.closePath();
        this.ctx.fillStyle = this.centerPressed ? 
            'rgba(255, 255, 255, 0.5)' : 'rgba(255, 255, 255, 0.2)';
        this.ctx.fill();
        this.ctx.strokeStyle = '#fff';
        this.ctx.stroke();

        // Draw center text
        this.ctx.fillStyle = '#fff';
        this.ctx.font = `${this.radius * 0.15}px Arial`;
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'middle';
        
        let buttonText = '';
        if (this.focusedSegment !== -1) {
            buttonText = this.ledStates[this.focusedSegment] ? 'OFF' : 'ON';
        } else {
            buttonText = this.ledStates.some(state => state) ? 'ALL OFF' : 'ALL ON';
        }
        
        this.ctx.fillText(buttonText, this.center, this.center);

        // フォーカスされている面のフォーカスラインを描画
        if (this.focusedSegment !== -1) {
            this.drawFocusLine(this.focusedSegment);
        }
    }
}

// 接続ステータスの更新
function updateConnectionStatus(status) {
    const statusIndicator = document.querySelector('.status-indicator');
    const statusText = document.querySelector('.status-text');
    
    connectionStatus = status;
    
    if (status === 'connected') {
        statusIndicator.classList.add('connected');
        statusText.textContent = '接続済み';
    } else if (status === 'disconnected') {
        statusIndicator.classList.remove('connected');
        statusText.textContent = '切断';
    } else {
        statusIndicator.classList.remove('connected');
        statusText.textContent = '接続中...';
    }
}

// APIステータスの確認
function checkApiStatus() {
    fetch('/api/status')
        .then(response => {
            if (response.ok) {
                updateConnectionStatus('connected');
                return response.json();
            } else {
                throw new Error('API status check failed');
            }
        })
        .then(data => {
            console.log('Device status:', data);
            
            // 定期的にステータスを確認
            setTimeout(checkApiStatus, 10000);
        })
        .catch(error => {
            console.error('Error checking API status:', error);
            updateConnectionStatus('disconnected');
            
            // 再接続を試みる
            setTimeout(checkApiStatus, 5000);
        });
}

// パターンリストの取得
function fetchPatterns() {
    fetch('/api/led/patterns')
        .then(response => {
            if (response.ok) {
                return response.json();
            } else {
                throw new Error('Failed to fetch patterns');
            }
        })
        .then(data => {
            patterns = data.patterns || [];
            console.log('Patterns loaded:', patterns);
        })
        .catch(error => {
            console.error('Error fetching patterns:', error);
            showStatus('パターンの取得に失敗しました', 'danger');
        });
}

// パターンモーダルを表示
function showPatternModal() {
    // 既存のモーダルがあれば削除
    let existingModal = document.getElementById('patternModal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // モーダルを作成
    const modal = document.createElement('div');
    modal.id = 'patternModal';
    modal.className = 'modal fade show';
    modal.style.display = 'block';
    modal.style.position = 'fixed';
    modal.style.top = '0';
    modal.style.left = '0';
    modal.style.width = '100%';
    modal.style.height = '100%';
    modal.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
    modal.style.zIndex = '1050';
    modal.style.overflow = 'auto';
    
    // モーダルの内容を作成
    let modalContent = `
        <div class="modal-dialog modal-dialog-centered" style="position: relative; width: auto; margin: 1.75rem auto; max-width: 500px;">
            <div class="modal-content" style="position: relative; background-color: var(--bs-dark); border: 1px solid rgba(0, 0, 0, 0.2); border-radius: 0.3rem; outline: 0;">
                <div class="modal-header" style="display: flex; align-items: center; justify-content: space-between; padding: 1rem; border-bottom: 1px solid #dee2e6;">
                    <h5 class="modal-title">LEDパターン選択</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close" style="background: transparent; border: 0; font-size: 1.5rem; color: white; cursor: pointer;">×</button>
                </div>
                <div class="modal-body" style="position: relative; padding: 1rem;">
                    <div class="list-group">
    `;
    
    // パターンリストを追加
    if (patterns.length > 0) {
        patterns.forEach(pattern => {
            modalContent += `
                <button type="button" class="list-group-item list-group-item-action pattern-item" data-pattern-id="${pattern.id}" style="position: relative; display: block; padding: 0.75rem 1.25rem; background-color: var(--bs-dark); color: white; border: 1px solid rgba(255, 255, 255, 0.1); cursor: pointer;">
                    ${pattern.name}
                </button>
            `;
        });
    } else {
        modalContent += `
            <div class="alert alert-info">
                パターンがありません
            </div>
        `;
    }
    
    // モーダルのフッターを追加
    modalContent += `
                    </div>
                </div>
                <div class="modal-footer" style="display: flex; align-items: center; justify-content: flex-end; padding: 1rem; border-top: 1px solid #dee2e6; gap: 0.5rem;">
                    <button type="button" class="btn btn-danger" id="stopPatternBtn">停止</button>
                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">閉じる</button>
                </div>
            </div>
        </div>
    `;
    
    modal.innerHTML = modalContent;
    document.body.appendChild(modal);
    
    // 背景をクリックしたらモーダルを閉じる
    modal.addEventListener('click', (e) => {
        if (e.target === modal) {
            closePatternModal();
        }
    });
    
    // 閉じるボタンのイベント
    const closeBtn = modal.querySelector('.btn-close');
    closeBtn.addEventListener('click', closePatternModal);
    
    const closeModalBtn = modal.querySelector('[data-bs-dismiss="modal"]');
    closeModalBtn.addEventListener('click', closePatternModal);
    
    // パターン項目のクリックイベント
    const patternItems = modal.querySelectorAll('.pattern-item');
    patternItems.forEach(item => {
        item.addEventListener('click', () => {
            const patternId = item.dataset.patternId;
            runPattern(patternId);
            closePatternModal();
        });
    });
    
    // パターン停止ボタンのイベント
    const stopPatternBtn = document.getElementById('stopPatternBtn');
    stopPatternBtn.addEventListener('click', () => {
        stopPattern();
        closePatternModal();
    });
}

// パターンモーダルを閉じる
function closePatternModal() {
    const modal = document.getElementById('patternModal');
    if (modal) {
        modal.remove();
    }
}

// パターンを実行
function runPattern(patternId) {
    fetch(`/api/led/pattern/${patternId}`, {
        method: 'POST'
    })
    .then(response => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Failed to run pattern');
        }
    })
    .then(data => {
        console.log('Pattern running:', data);
        showStatus(`パターン「${patterns.find(p => p.id == patternId)?.name || patternId}」を実行中`, 'success');
    })
    .catch(error => {
        console.error('Error running pattern:', error);
        showStatus('パターンの実行に失敗しました', 'danger');
    });
}

// パターンを停止
function stopPattern() {
    fetch('/api/led/stop', {
        method: 'POST'
    })
    .then(response => {
        if (response.ok) {
            return response.json();
        } else {
            throw new Error('Failed to stop pattern');
        }
    })
    .then(data => {
        console.log('Pattern stopped:', data);
        showStatus('パターンを停止しました', 'success');
    })
    .catch(error => {
        console.error('Error stopping pattern:', error);
        showStatus('パターンの停止に失敗しました', 'danger');
    });
}

// ステータスメッセージを表示
function showStatus(message, type) {
    const statusEl = document.getElementById('statusMessage');
    statusEl.textContent = message;
    statusEl.className = `alert alert-${type}`;
    statusEl.classList.remove('d-none');

    setTimeout(() => {
        statusEl.classList.add('d-none');
    }, 3000);
}

// カラーピッカーモーダルを表示
function showColorPickerModal() {
    // 既存のモーダルがあれば削除
    let existingModal = document.getElementById('colorPickerModal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // モーダルを作成
    const modal = document.createElement('div');
    modal.id = 'colorPickerModal';
    modal.className = 'modal fade show';
    modal.style.display = 'block';
    modal.style.position = 'fixed';
    modal.style.top = '0';
    modal.style.left = '0';
    modal.style.width = '100%';
    modal.style.height = '100%';
    modal.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
    modal.style.zIndex = '1050';
    modal.style.overflow = 'auto';
    
    // モーダルの内容を作成
    modal.innerHTML = `
        <div class="modal-dialog modal-dialog-centered" style="position: relative; width: auto; margin: 1.75rem auto; max-width: 500px;">
            <div class="modal-content" style="position: relative; background-color: var(--bs-dark); border: 1px solid rgba(0, 0, 0, 0.2); border-radius: 0.3rem; outline: 0;">
                <div class="modal-header" style="display: flex; align-items: center; justify-content: space-between; padding: 1rem; border-bottom: 1px solid #dee2e6;">
                    <h5 class="modal-title">色を選択</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close" style="background: transparent; border: 0; font-size: 1.5rem; color: white; cursor: pointer;">×</button>
                </div>
                <div class="modal-body" style="position: relative; padding: 1rem;">
                    <div class="mb-3">
                        <label for="colorPicker" class="form-label">カラーピッカー</label>
                        <input type="color" class="form-control form-control-color w-100" id="colorPicker" value="${currentColorHex}">
                    </div>
                    <div class="color-presets d-flex flex-wrap gap-2 mb-3">
                        <button class="color-preset" style="background-color: #ff0000; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ff0000"></button>
                        <button class="color-preset" style="background-color: #00ff00; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#00ff00"></button>
                        <button class="color-preset" style="background-color: #0000ff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#0000ff"></button>
                        <button class="color-preset" style="background-color: #ffff00; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ffff00"></button>
                        <button class="color-preset" style="background-color: #00ffff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#00ffff"></button>
                        <button class="color-preset" style="background-color: #ff00ff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ff00ff"></button>
                        <button class="color-preset" style="background-color: #ffffff; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ffffff"></button>
                        <button class="color-preset" style="background-color: #ff8000; width: 40px; height: 40px; border-radius: 50%; border: 2px solid #fff; cursor: pointer;" data-color="#ff8000"></button>
                    </div>
                </div>
                <div class="modal-footer" style="display: flex; align-items: center; justify-content: flex-end; padding: 1rem; border-top: 1px solid #dee2e6; gap: 0.5rem;">
                    <button type="button" class="btn btn-primary" id="applyColorBtn">適用</button>
                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">キャンセル</button>
                </div>
            </div>
        </div>
    `;
    
    document.body.appendChild(modal);
    
    // 背景をクリックしたらモーダルを閉じる
    modal.addEventListener('click', (e) => {
        if (e.target === modal) {
            closeColorPickerModal();
        }
    });
    
    // 閉じるボタンのイベント
    const closeBtn = modal.querySelector('.btn-close');
    closeBtn.addEventListener('click', closeColorPickerModal);
    
    const closeModalBtn = modal.querySelector('[data-bs-dismiss="modal"]');
    closeModalBtn.addEventListener('click', closeColorPickerModal);
    
    // カラーピッカーのイベント
    const colorPicker = document.getElementById('colorPicker');
    colorPicker.addEventListener('input', (e) => {
        // 選択された色を保存
        currentColorHex = e.target.value;
    });
    
    // 色プリセットボタンのイベント
    const colorPresets = modal.querySelectorAll('.color-preset');
    colorPresets.forEach(preset => {
        preset.addEventListener('click', () => {
            const color = preset.dataset.color;
            currentColorHex = color;
            colorPicker.value = color;
        });
    });
    
    // 適用ボタンのイベント
    const applyBtn = document.getElementById('applyColorBtn');
    applyBtn.addEventListener('click', () => {
        // 選択された色を適用して閉じる
        closeColorPickerModal();
        showStatus('色を変更しました: ' + currentColorHex, 'success');
    });
}

// カラーピッカーモーダルを閉じる
function closeColorPickerModal() {
    const modal = document.getElementById('colorPickerModal');
    if (modal) {
        modal.remove();
    }
}

// モーダル用のスタイルを追加
function addModalStyles() {
    // 既存のスタイルがあれば削除
    let existingStyle = document.getElementById('modalStyles');
    if (existingStyle) {
        existingStyle.remove();
    }
    
    // スタイルを作成
    const style = document.createElement('style');
    style.id = 'modalStyles';
    style.textContent = `
        .modal {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
        }
        .modal-title {
            color: white;
            margin: 0;
        }
        .form-label {
            color: white;
            margin-bottom: 0.5rem;
            display: block;
        }
        .form-control-color {
            height: 40px;
            padding: 0.375rem;
            background-color: #343a40;
            border: 1px solid #6c757d;
            border-radius: 0.25rem;
        }
        .color-preset {
            transition: transform 0.2s;
        }
        .color-preset:hover {
            transform: scale(1.1);
        }
        .list-group-item-action:hover {
            background-color: rgba(255, 255, 255, 0.1) !important;
        }
    `;
    
    document.head.appendChild(style);
}

// JSONパターンモーダルを表示
function showJsonPatternModal() {
    // 既存のモーダルがあれば削除
    let existingModal = document.getElementById('jsonPatternModal');
    if (existingModal) {
        existingModal.remove();
    }
    
    // モーダルを作成
    const modal = document.createElement('div');
    modal.id = 'jsonPatternModal';
    modal.className = 'modal fade show';
    modal.style.display = 'block';
    modal.style.position = 'fixed';
    modal.style.top = '0';
    modal.style.left = '0';
    modal.style.width = '100%';
    modal.style.height = '100%';
    modal.style.backgroundColor = 'rgba(0, 0, 0, 0.5)';
    modal.style.zIndex = '1050';
    modal.style.overflow = 'auto';
    
    // モーダルの内容を作成
    modal.innerHTML = `
        <div class="modal-dialog modal-dialog-centered modal-lg" style="position: relative; width: auto; margin: 1.75rem auto; max-width: 800px;">
            <div class="modal-content" style="position: relative; background-color: var(--bs-dark); border: 1px solid rgba(0, 0, 0, 0.2); border-radius: 0.3rem; outline: 0;">
                <div class="modal-header" style="display: flex; align-items: center; justify-content: space-between; padding: 1rem; border-bottom: 1px solid #dee2e6;">
                    <h5 class="modal-title">JSONパターン設定</h5>
                    <button type="button" class="btn-close" data-bs-dismiss="modal" aria-label="Close" style="background: transparent; border: 0; font-size: 1.5rem; color: white; cursor: pointer;">×</button>
                </div>
                <div class="modal-body" style="position: relative; padding: 1rem;">
                    <div class="mb-3">
                        <label for="jsonPatternEditor" class="form-label">JSONパターン</label>
                        <textarea id="jsonPatternEditor" class="form-control" style="height: 400px; background-color: #343a40; color: white; font-family: monospace;">${jsonPatternTemplate}</textarea>
                    </div>
                </div>
                <div class="modal-footer" style="display: flex; align-items: center; justify-content: flex-end; padding: 1rem; border-top: 1px solid #dee2e6; gap: 0.5rem;">
                    <button type="button" class="btn btn-primary" id="applyJsonBtn">適用</button>
                    <button type="button" class="btn btn-secondary" data-bs-dismiss="modal">キャンセル</button>
                </div>
            </div>
        </div>
    `;
    
    document.body.appendChild(modal);
    
    // 背景をクリックしたらモーダルを閉じる
    modal.addEventListener('click', (e) => {
        if (e.target === modal) {
            closeJsonPatternModal();
        }
    });
    
    // 閉じるボタンのイベント
    const closeBtn = modal.querySelector('.btn-close');
    closeBtn.addEventListener('click', closeJsonPatternModal);
    
    // キャンセルボタンのイベント
    const cancelBtn = modal.querySelector('[data-bs-dismiss="modal"]');
    if (cancelBtn) {
        cancelBtn.addEventListener('click', closeJsonPatternModal);
    }
    
    // 適用ボタンのイベント
    const applyBtn = document.getElementById('applyJsonBtn');
    applyBtn.addEventListener('click', () => {
        // JSONパターンを適用
        applyJsonPattern();
    });
}

// JSONパターンモーダルを閉じる
function closeJsonPatternModal() {
    const modal = document.getElementById('jsonPatternModal');
    if (modal) {
        modal.remove();
    }
}

// JSONパターンを適用
function applyJsonPattern() {
    const jsonEditor = document.getElementById('jsonPatternEditor');
    const jsonText = jsonEditor.value;
    
    try {
        // JSONの検証
        const patternObj = JSON.parse(jsonText);

        // APIにJSONパターンを送信
        fetch('/api/led/pattern/json', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json'
            },
            body: jsonText // 直接送信
        })
        .then(response => {
            if (response.ok) {
                return response.json();
            } else {
                throw new Error('Failed to apply JSON pattern');
            }
        })
        .then(data => {
            console.log('JSON pattern applied:', data);
            showStatus('JSONパターンを適用しました', 'success');
            closeJsonPatternModal();
        })
        .catch(error => {
            console.error('Error applying JSON pattern:', error);
            showStatus('JSONパターンの適用に失敗しました', 'danger');
        });
    } catch (error) {
        console.error('Invalid JSON:', error);
        showStatus('無効なJSONフォーマットです', 'danger');
    }
}
