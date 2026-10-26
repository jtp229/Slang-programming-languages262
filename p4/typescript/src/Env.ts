import type { Value } from './Values';

/**
 * Env is the environment: a mapping from names to values.
 *
 * NB: For this assignment, there are no lambdas or let, so we don't need nested
 * environments.
 */
export class Env {
  /** The mapping from names to expressions */
  private map: Map<string, Value> = new Map<string, Value>();

  /**
   * Add a key/value pair to the Environment
   *
   * @param key  The string name
   * @param expr The expression that serves as the value
   */
  public put(key: string, expr: Value) {
    this.map.set(key, expr);
  }

  /**
   * Get the value for a given key.
   *
   * @param key The key to look up
   * @return The value associated with the key, or undefined
   */
  public get(key: string): Value | undefined {
    return this.map.get(key);
  }
}
