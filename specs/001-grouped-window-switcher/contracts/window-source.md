# Contract: WindowSource (core ⇄ compositor seam)

The single boundary between behavioral logic and the live compositor. The core depends only on this
interface, so all logic is testable with a fake. The live adapter (`EffectWindowSource`) wraps KWin;
the test adapter (`FakeWindowSource`) is scripted.

## Interface (conceptual)

```cpp
class WindowSource : public QObject {
public:
    // Eligible windows (FR-001/017/020), filtered to user-facing top-level windows,
    // across all virtual desktops, activities, and monitors.
    virtual QList<WindowDescriptor> snapshot() const = 0;

    // Activate a window: raise, focus, and switch to its desktop/activity if needed (FR-010/020).
    // Returns false if the window no longer exists (FR-018 graceful fallback).
    virtual bool activate(const WindowId &id) = 0;

    // Restore focus to a previously-active window on cancel (FR-009).
    virtual void restoreActive(const WindowId &id) = 0;

    // The window active immediately before the switcher opened (for cancel/restore + default highlight).
    virtual WindowId previouslyActive() const = 0;

Q_SIGNALS:
    void windowAdded(const WindowDescriptor &);    // FR-018
    void windowRemoved(const WindowId &);          // FR-018
    void windowChanged(const WindowDescriptor &);  // caption/minimized/desktop updates
};
```

## Guarantees

- `snapshot()` returns descriptors already filtered for eligibility (no panels/docks/overlay).
- Descriptors are immutable value copies — safe to hold and reason about without locking.
- `windowHandle` in each descriptor is a valid live window at runtime, `null` under test.
- The live adapter performs **no blocking calls** on the compositor thread (Principle II).

## Test double (`FakeWindowSource`)

- Constructed from a scripted list of `WindowDescriptor`s with controllable `appKey`, `lastUsedAt`,
  `isMinimized`, `desktop`, etc.
- Can emit `windowAdded/Removed/Changed` on demand to drive live-update tests (FR-018).
- `activate()` records the requested id for assertions; never touches a compositor.

## Maps to requirements

FR-001, FR-005, FR-010, FR-017, FR-018, FR-019, FR-020; Principle III (testable seam).
