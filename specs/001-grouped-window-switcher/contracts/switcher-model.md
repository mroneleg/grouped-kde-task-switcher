# Contract: SwitcherModel & Controller (QML-facing API)

The QML overlay consumes a model + a controller. The model is read-only data; the controller
receives intent (navigation/confirm/cancel) and mutates `SessionController` state. Neither calls the
compositor directly — activation goes through `WindowSource`.

## SwitcherModel (groups + entries)

A two-level model (groups → window entries) exposed to QML. Implemented as a `QAbstractItemModel`
(or a group list each holding an entry list).

**Group roles** (Overview — `GroupOverview.qml`):

| Role | Type | Requirement |
|------|------|-------------|
| `appDisplayName` | string | FR-002 |
| `appIconName` | string | FR-002 |
| `windowCount` | int | FR-002 |
| `isFallback` | bool | FR-017 |

**Entry roles** (In-group grid — `ApplicationGroupGrid.qml` / `WindowTile.qml`):

| Role | Type | Requirement |
|------|------|-------------|
| `caption` | string | FR-005 |
| `windowHandle` | object (live KWin window; null in tests) | binds `WindowThumbnail.client`; FR-011 |
| `thumbnailState` | enum (`Warming`/`Ready`/`IconFallback`) | FR-011/012 |
| `appIconName` | string | icon fallback for minimized/no-texture; FR-012 |

**Guarantees**: ordering is MRU and stable (FR-003/006); the model updates on `WindowSource`
change signals and emits the proper Qt model-reset/row signals so the view stays consistent
(FR-018); when empty, exposes an `isEmpty` flag for the empty state (FR-019).

## SwitcherController (intent API)

Invoked by QML key/mouse handlers. Pure state transitions over `SessionController`; see
[interaction-state-machine.md](./interaction-state-machine.md).

```cpp
class SwitcherController : public QObject {
public:                                   // Q_INVOKABLE for QML
    void open();                          // shortcut → Overview; build groups; warm thumbnails (FR-008/011)
    void advanceHighlight();              // shortcut re-press / forward key — wraps (FR-009/004)
    void retreatHighlight();              // reverse navigation (FR-004)
    void moveHighlight(Direction d);      // arrow keys in overview or grid (FR-004/006)
    void enterHighlightedGroup();         // Overview → InGroup (FR-005)
    void back();                          // InGroup → Overview, keep origin highlighted (FR-005)
    void confirm();                       // activate selection, close (FR-010)
    void cancel();                        // Escape → restore previous active, close (FR-009)
    void setScrollOffset(int);            // grid scrolling state (FR-007)

Q_SIGNALS:
    void stateChanged();                  // QML re-reads level/highlight/openGroup
    void closed();                        // effect tears down overlay + ungrabs keyboard
};
```

**Exposed read state** (properties for QML bindings): `level`, `highlightedGroupIndex`,
`openGroupIndex`, `highlightedWindowIndex`, `isEmpty`.

## Maps to requirements

FR-002 through FR-012, FR-017, FR-018, FR-019; Principle III (controller is compositor-free,
unit-tested via `FakeWindowSource`).
